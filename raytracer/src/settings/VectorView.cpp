#include "VectorView.hpp"

#include <QDebug>

#include <algorithm>
#include <math.h>

namespace Rt {

    VectorView::VectorView(int n, QBoxLayout::Direction dir, QWidget* parent) : QWidget(parent), n(n) {
        Q_ASSERT_X(n >= 1, "VecSpinBox Initialization", "Cannot have a zero-sized VecSpinBoxes");
        data = new float[n];

        QBoxLayout* layout = new QBoxLayout(dir, this);

        spinboxes = new QDoubleSpinBox[n];
        spinbox_decimals = 2;
        for (int i=0; i<n; i++) {
            layout->addWidget(&spinboxes[i]);
            spinboxes[i].setMaximum(100);
            spinboxes[i].setMinimum(-100);
            spinboxes[i].setSingleStep(0.05);
            connect(
                &spinboxes[i],
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this,
                [=](double v){
                    set_value_at(v, i);
                }
            );
        }
    }

    VectorView::~VectorView() {
        delete[] data;
        delete[] spinboxes;
    }

    int VectorView::dimensions() const {
        return n;
    }

    void VectorView::set_value(const float* value) {
        bool data_changed = false;
        float epsilon = std::pow(9/10, spinbox_decimals+1);
        for (int i=0; i<n; i++) {
            if (abs(data[i]-value[i]) > epsilon) {
                data[i] = value[i];
                spinboxes[i].setValue(data[i]);
                data_changed = true;
            }
        }
        if (data_changed)
            emit value_changed(data, n);
    }

    void VectorView::set_value_at(float value, int i) {
        float epsilon = std::pow(9/10, spinbox_decimals+1);
        if (abs(data[i] - value) > epsilon) {
            data[i] = value;
            spinboxes[i].setValue(data[i]);
            emit value_changed(data, n);
        }
    }

    const float* VectorView::value() const {
        return data;
    }

    void VectorView::set_decimals(int dec) {
        spinbox_decimals = dec;
        for (int i=0; i<n; i++) {
            spinboxes[i].setDecimals(dec);
        }
    }

    int VectorView::decimals() const {
        return spinbox_decimals;
    }

}