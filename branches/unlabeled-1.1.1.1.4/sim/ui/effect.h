#ifndef _EFFECT_H
#define _EFFECT_H 1

enum GradientOrientation
{
    VerticalGradient,
    HorizontalGradient
};

void gradient(QPixmap &pict, const QColor &ca,
              const QColor &cb, GradientOrientation eff);

QImage& fade(QImage &pict, float val, const QColor &color);

QPixmap& intensity(QPixmap &pict, float percent);

#endif

