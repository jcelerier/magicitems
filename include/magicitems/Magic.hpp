#pragma once
#if !defined(NDEBUG)
#include <QSpinBox>
#include <QGraphicsItem>
#include <QHash>
#include <QFormLayout>
#include <unordered_map>

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
#else
class MagicGraphicsItem
{
  static constexpr double magic(const char*, double defval, double, double) noexcept { return defval; }
  static constexpr QPointF magic(const char*, QPointF defval, QPointF, QPointF) noexcept { return defval; }
};
#endif