#include <qfile.h>
#include <qimage.h>
#include <qpixmap.h>
#include "effect.h"

void gradient(QPixmap &pict, const QColor &ca,
              const QColor &cb, GradientOrientation eff)
{
    QSize size = pict.size();

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image(size, 32);

    if (size.width() == 0 || size.height() == 0)
        return;

    register int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    uint *p;
    uint rgb;

    register int rl = rca << 16;
    register int gl = gca << 16;
    register int bl = bca << 16;

    if( eff == VerticalGradient ) {

        int rcdelta = ((1<<16) / size.height()) * rDiff;
        int gcdelta = ((1<<16) / size.height()) * gDiff;
        int bcdelta = ((1<<16) / size.height()) * bDiff;

        for ( y = 0; y < size.height(); y++ ) {
            p = (uint *) image.scanLine(y);

            rl += rcdelta;
            gl += gcdelta;
            bl += bcdelta;

            rgb = qRgb( (rl>>16), (gl>>16), (bl>>16) );

            for( x = 0; x < size.width(); x++ ) {
                *p = rgb;
                p++;
            }
        }
    } else {                  // must be HorizontalGradient

        unsigned int *o_src = (unsigned int *)image.scanLine(0);
        unsigned int *src = o_src;

        int rcdelta = ((1<<16) / size.width()) * rDiff;
        int gcdelta = ((1<<16) / size.width()) * gDiff;
        int bcdelta = ((1<<16) / size.width()) * bDiff;

        for( x = 0; x < size.width(); x++) {

            rl += rcdelta;
            gl += gcdelta;
            bl += bcdelta;

            *src++ = qRgb( (rl>>16), (gl>>16), (bl>>16));
        }

        src = o_src;

        // Believe it or not, manually copying in a for loop is faster
        // than calling memcpy for each scanline (on the order of ms...).
        // I think this is due to the function call overhead (mosfet).

        for (y = 1; y < size.height(); ++y) {

            p = (unsigned int *)image.scanLine(y);
            src = o_src;
            for(x=0; x < size.width(); ++x)
                *p++ = *src++;
            *p++ = *src++;
        }
    }
    pict.convertFromImage(image);
}

QImage& fade(QImage &img, float val, const QColor &color)
{
    if (img.width() == 0 || img.height() == 0)
        return img;

    unsigned char tbl[256];
    for (int i=0; i<256; i++)
        tbl[i] = (int) (val * i + 0.5);

    int red = color.red();
    int green = color.green();
    int blue = color.blue();

    QRgb col;
    int r, g, b, cr, cg, cb;

    if (img.depth() <= 8) {
        // pseudo color
        for (int i=0; i<img.numColors(); i++) {
            col = img.color(i);
            cr = qRed(col); cg = qGreen(col); cb = qBlue(col);
            if (cr > red)
                r = cr - tbl[cr - red];
            else
                r = cr + tbl[red - cr];
            if (cg > green)
                g = cg - tbl[cg - green];
            else
                g = cg + tbl[green - cg];
            if (cb > blue)
                b = cb - tbl[cb - blue];
            else
                b = cb + tbl[blue - cb];
            img.setColor(i, qRgb(r, g, b));
        }

    } else {
        // truecolor
        for (int y=0; y<img.height(); y++) {
            QRgb *data = (QRgb *) img.scanLine(y);
            for (int x=0; x<img.width(); x++) {
                col = *data;
                cr = qRed(col); cg = qGreen(col); cb = qBlue(col);
                cr = qRed(col); cg = qGreen(col); cb = qBlue(col);
                if (cr > red)
                    r = cr - tbl[cr - red];
                else
                    r = cr + tbl[red - cr];
                if (cg > green)
                    g = cg - tbl[cg - green];
                else
                    g = cg + tbl[green - cg];
                if (cb > blue)
                    b = cb - tbl[cb - blue];
                else
                    b = cb + tbl[blue - cb];
                *data++ = qRgb(r, g, b);
            }
        }
    }
    return img;
}

QPixmap& intensity(QPixmap &pict, float percent)
{
    QImage image = pict.convertToImage();
    int i, tmp, r, g, b;
    int segColors = image.depth() > 8 ? 256 : image.numColors();
    unsigned char *segTbl = new unsigned char[segColors];
    int pixels = image.depth() > 8 ? image.width()*image.height() :
                 image.numColors();
    unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();

    bool brighten = (percent >= 0);
    if(percent < 0)
        percent = -percent;

    if(brighten){ // keep overflow check out of loops
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp > 255)
                tmp = 255;
            segTbl[i] = tmp;
        }
    }
    else{
        for(i=0; i < segColors; ++i){
            tmp = (int)(i*percent);
            if(tmp < 0)
                tmp = 0;
            segTbl[i] = tmp;
        }
    }

    if(brighten){ // same here
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
            g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
            b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
    else{
        for(i=0; i < pixels; ++i){
            r = qRed(data[i]);
            g = qGreen(data[i]);
            b = qBlue(data[i]);
            r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
            g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
            b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
            data[i] = qRgb(r, g, b);
        }
    }
    delete [] segTbl;

    pict.convertFromImage(image);
    return pict;
}



