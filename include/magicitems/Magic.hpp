#pragma once
#if !defined(NDEBUG)
#include <QSpinBox>
#include <QGraphicsItem>
#include <QHash>
#include <QFormLayout>
#include <unordered_map>
#include <QFrame>

namespace std
{
template <>
struct hash<std::pair<QString, QGraphicsItem*>>
{
  std::size_t operator()(const std::pair<QString, QGraphicsItem*>& p) const noexcept
  {
    return qHash(p);
  }
};
}

class MagicWidget : public QWidget
{
public:
  MagicWidget()
  {
    setLayout(new QFormLayout);
    show();
  }

  struct Connection
  {
    QWidget* widget{};
  };

  std::unordered_map<std::pair<QString, QGraphicsItem*>, Connection> items;

  static MagicWidget& instance()
  {
    static MagicWidget w;
    return w;
  }
};

class MagicGraphicsItem
{
public:
  QObject context;
  MagicGraphicsItem()
  {

  }

  virtual ~MagicGraphicsItem()
  {

  }

  double magic(QString name, double defval, double min, double max, std::function<void(double)> f = {})
  {
    auto& m = MagicWidget::instance();

    auto item = dynamic_cast<QGraphicsItem*>(this);
    auto ctx = &context;

    if(auto it = m.items.find({name, item}); it != m.items.end())
    {
      return ((QSpinBox*)it->second.widget)->value();
    }
    else
    {
      if(!f)
        f = [=] (double) { item->update(); };

      auto widg = new QSpinBox;
      widg->setRange(min, max);
      widg->setValue(defval);
      QObject::connect(widg, qOverload<int>(&QSpinBox::valueChanged), ctx, f);
      static_cast<QFormLayout*>(m.layout())->addRow(name, widg);

      m.items[{name, item}] = { widg };

      QObject::connect(ctx, &QObject::destroyed, widg, [=] {
        auto& m = MagicWidget::instance();
        auto widg = m.items[{name, item}].widget;
        static_cast<QFormLayout*>(m.layout())->removeRow(widg);
        widg->deleteLater();
        m.items.erase({name, item});
      });
      return defval;
    }
  }

  QPointF magic(QString name, QPointF defval, QPointF min, QPointF max, std::function<void(QPointF)> f = {})
  {
    auto& m = MagicWidget::instance();

    auto item = dynamic_cast<QGraphicsItem*>(this);
    auto ctx = &context;

    if(auto it = m.items.find({name, item}); it != m.items.end())
    {
      auto x = (QSpinBox*)it->second.widget->children()[1];
      auto y = (QSpinBox*)it->second.widget->children()[2];

      return QPointF(x->value(), y->value());
    }
    else
    {
      if(!f)
      {
        f = [=] (QPointF) { item->update(); };
      }

      auto widg = new QWidget;
      {
        auto lay = new QHBoxLayout;
        widg->setLayout(lay);
        auto x = new QSpinBox;
        auto y = new QSpinBox;

        x->setRange(min.x(), max.x());
        x->setValue(defval.x());
        QObject::connect(x, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int x) { f(QPointF(x, y->value())); });
        lay->addWidget(x);

        y->setRange(min.y(), max.y());
        y->setValue(defval.y());
        QObject::connect(y, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int y) { f(QPointF(x->value(), y)); });
        lay->addWidget(y);
      }
      static_cast<QFormLayout*>(m.layout())->addRow(name, widg);

      m.items[{name, item}] = MagicWidget::Connection{widg};

      QObject::connect(ctx, &QObject::destroyed, widg, [=] {
        auto& m = MagicWidget::instance();
        auto widg = m.items[{name, item}].widget;
        static_cast<QFormLayout*>(m.layout())->removeRow(widg);
        widg->deleteLater();
        m.items.erase({name, item});
      });
      return defval;
    }
  }
};

static void magic_pos(QString name, QGraphicsItem* item, QObject* ctx)
{
  auto& m = MagicWidget::instance();

  auto widg = new QWidget;
  {
    auto lay = new QHBoxLayout;
    widg->setLayout(lay);
    auto x = new QSpinBox;
    auto y = new QSpinBox;

    x->setRange(-30, 500);
    x->setValue(item->pos().x());
    QObject::connect(x, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int x) { item->setPos(x, y->value()); });
    lay->addWidget(x);

    y->setRange(-30, 500);
    y->setValue(item->pos().y());
    QObject::connect(y, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int y) { item->setPos(x->value(), y); });
    lay->addWidget(y);
  }
  static_cast<QFormLayout*>(m.layout())->addRow(name, widg);

  m.items[{name, item}] = MagicWidget::Connection{widg};
/*
  QObject::connect(ctx, &QObject::destroyed, widg, [=] {
    auto& m = MagicWidget::instance();
    auto widg = m.items[{name, item}].widget;
    static_cast<QFormLayout*>(m.layout())->removeRow(widg);
    widg->deleteLater();
    m.items.erase({name, item});
  });*/
}

template<typename T>
static void magic_rect(QString name, T* item, QObject* ctx)
{
  auto& m = MagicWidget::instance();

  auto widg = new QFrame;
  widg->setFrameStyle(QFrame::Box);
  {
    auto lay = new QHBoxLayout;
    widg->setLayout(lay);
    auto x = new QSpinBox;
    auto y = new QSpinBox;
    auto w = new QSpinBox;
    auto h = new QSpinBox;

    x->setRange(-30, 500);
    x->setValue(item->pos().x());
    QObject::connect(x, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int x) { item->setPos(x, y->value()); });
    lay->addWidget(x);

    y->setRange(-30, 500);
    y->setValue(item->pos().y());
    QObject::connect(y, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int y) { item->setPos(x->value(), y); });
    lay->addWidget(y);

    w->setRange(10, 500);
    w->setValue(item->boundingRect().width());
    QObject::connect(w, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int w) {
      auto rect = item->boundingRect();
      rect.setWidth(w);
      item->setRect(rect);
    });
    lay->addWidget(w);

    h->setRange(10, 500);
    h->setValue(item->boundingRect().height());
    QObject::connect(h, qOverload<int>(&QSpinBox::valueChanged), ctx,  [=] (int h) {
      auto rect = item->boundingRect();
      rect.setHeight(h);
      item->setRect(rect);
    });
    lay->addWidget(h);
  }
  static_cast<QFormLayout*>(m.layout())->addRow(name, widg);

  m.items[{name, item}] = MagicWidget::Connection{widg};
/*
  QObject::connect(ctx, &QObject::destroyed, widg, [=] {
    auto& m = MagicWidget::instance();
    auto widg = m.items[{name, item}].widget;
    static_cast<QFormLayout*>(m.layout())->removeRow(widg);
    widg->deleteLater();
    m.items.erase({name, item});
  });
  */
}
#else
struct MagicGraphicsItem
{
  template<typename... F>
  static constexpr double magic(const char*, double defval, double, double, F&&...) noexcept { return defval; }
  
  template<typename... F>
  static constexpr QPointF magic(const char*, QPointF defval, QPointF, QPointF, F&&...) noexcept { return defval; }
};
#endif
