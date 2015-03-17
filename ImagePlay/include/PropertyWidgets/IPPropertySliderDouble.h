#ifndef IPPROPERTYSLIDERDOUBLE_H
#define IPPROPERTYSLIDERDOUBLE_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QDebug>

#include "IPPropertyWidget.h"

class IPPropertySliderDouble : public IPPropertyWidget
{
    Q_OBJECT
public:
    IPPropertySliderDouble(IPLProcessPropertyDouble* property, QWidget *parent) : IPPropertyWidget(parent)
    {
        setLayout(new QHBoxLayout);
        layout()->setMargin(0);

        _property = NULL;

        double min   = ((IPLProcessPropertyDouble*) property)->min();
        double max   = ((IPLProcessPropertyDouble*) property)->max();
        double value = ((IPLProcessPropertyDouble*) property)->value();

        _slider = new QSlider(Qt::Horizontal, this);
        _spinner = new QDoubleSpinBox(this);

        layout()->addWidget(_slider);
        layout()->addWidget(_spinner);

        connect(_slider, &QSlider::sliderReleased, this, &IPPropertySliderDouble::updateValue );
        connect(_slider, &QSlider::valueChanged, this, &IPPropertySliderDouble::updateSpinner );
        connect(_spinner, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &IPPropertySliderDouble::updateSlider );

        _slider->setMinimum(min*100);
        _slider->setMaximum(max*100);
        _slider->setValue(value*100);

        _spinner->setMinimum(min);
        _spinner->setMaximum(max);
        _spinner->setValue(value);
        _spinner->setSingleStep(0.01);

        _property = property;
    }
    void setMinimum(double v)  { _slider->setMinimum(v*100); }
    void setMaximum(double v)  { _slider->setMaximum(v*100); }
    double value()             { return _slider->value()/100.0; }

    void saveValue()        { _property->setValue(value()); }

signals:

public slots:
    void updateSpinner(int)
    {
        _spinner->setValue(value());

        updateValue();
    }

    void updateSlider(double v)
    {
        _slider->setValue(v*100);

        updateValue();
    }


    void updateValue()
    {
        // prevent double changes
        if(value() == _lastValue)
        {
            return;
        }

        if(_property)
            saveValue();

        emit changed();

        _lastValue = value();
    }

private:
    IPLProcessPropertyDouble* _property;
    QSlider*                    _slider;
    QDoubleSpinBox*             _spinner;
    double                      _lastValue;

};

#endif // IPPROPERTYSLIDERDOUBLE_H