/****************************************************************************
** $Id: qjpegio.cpp,v 1.3 2004-04-11 10:24:14 shutoff Exp $
**
** Implementation of JPEG QImage IOHandler
**
** Created : 990521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QT_CLEAN_NAMESPACE

#include "qjpegio.h"
#include "qfeatures.h"

#ifndef QT_NO_IMAGEIO_JPEG

#include <qimage.h>
#include <qiodevice.h>

#include <stdio.h> // jpeglib needs this to be pre-included
#include <setjmp.h>


// including jpeglib.h seems to be a little messy
extern "C" {
#define XMD_H // Shut JPEGlib up.
#if defined(_OS_UNIXWARE7_)
#  define HAVE_BOOLEAN // libjpeg under Unixware seems to need this
#  define HAVE_PROTOTYPES
#endif
#include <jpeglib.h>
#ifdef const
#  undef const // Remove crazy C hackery in jconfig.h
#endif
}


struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

    static
    void my_error_exit (j_common_ptr cinfo)
    {
        my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        qWarning(buffer);
        longjmp(myerr->setjmp_buffer, 1);
    }

#if defined(Q_C_CALLBACKS)
}
#endif


static const int max_buf = 4096;

struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(QImageIO* iio);
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

    static
    void qt_init_source(j_decompress_ptr)
    {
    }

    static
    boolean qt_fill_input_buffer(j_decompress_ptr cinfo)
    {
        int num_read;
        my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
        QIODevice* dev = src->iio->ioDevice();
        src->next_input_byte = src->buffer;
        num_read = dev->readBlock((char*)src->buffer, max_buf);
        if ( num_read <= 0 ) {
            // Insert a fake EOI marker - as per jpeglib recommendation
            src->buffer[0] = (JOCTET) 0xFF;
            src->buffer[1] = (JOCTET) JPEG_EOI;
            src->bytes_in_buffer = 2;
        } else {
            src->bytes_in_buffer = num_read;
        }
#if defined(_OS_UNIXWARE7_)
        return B_TRUE;
#else
        return TRUE;
#endif
    }

    static
    void qt_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
    {
        my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

        // `dumb' implementation from jpeglib

        /* Just a dumb implementation for now.  Could use fseek() except
         * it doesn't work on pipes.  Not clear that being smart is worth
         * any trouble anyway --- large skips are infrequent.
         */
        if (num_bytes > 0) {
            while (num_bytes > (long) src->bytes_in_buffer) {
                num_bytes -= (long) src->bytes_in_buffer;
                (void) qt_fill_input_buffer(cinfo);
                /* note we assume that qt_fill_input_buffer will never return FALSE,
                * so suspension need not be handled.
                */
            }
            src->next_input_byte += (size_t) num_bytes;
            src->bytes_in_buffer -= (size_t) num_bytes;
        }
    }

    static
    void qt_term_source(j_decompress_ptr)
    {
    }

#if defined(Q_C_CALLBACKS)
}
#endif


inline my_jpeg_source_mgr::my_jpeg_source_mgr(QImageIO* iioptr)
{
    jpeg_source_mgr::init_source = qt_init_source;
    jpeg_source_mgr::fill_input_buffer = qt_fill_input_buffer;
    jpeg_source_mgr::skip_input_data = qt_skip_input_data;
    jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
    jpeg_source_mgr::term_source = qt_term_source;
    iio = iioptr;
    bytes_in_buffer = 0;
    next_input_byte = buffer;
}


static
void read_jpeg_image(QImageIO* iio)
{
    QImage image;

    struct jpeg_decompress_struct cinfo;

    struct my_jpeg_source_mgr *iod_src = new my_jpeg_source_mgr(iio);
    struct my_error_mgr jerr;

    jpeg_create_decompress(&cinfo);

    cinfo.src = iod_src;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
#if defined(_OS_UNIXWARE7_)
        (void) jpeg_read_header(&cinfo, B_TRUE);
#else
        (void) jpeg_read_header(&cinfo, TRUE);
#endif

        (void) jpeg_start_decompress(&cinfo);

        if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
            image.create( cinfo.output_width, cinfo.output_height, 32 );
        } else if ( cinfo.output_components == 1 ) {
            image.create( cinfo.output_width, cinfo.output_height, 8, 256 );
            for (int i=0; i<256; i++)
                image.setColor(i, qRgb(i,i,i));
        } else {
            // Unsupported format
        }

        if (!image.isNull()) {
            uchar** lines = image.jumpTable();
            while (cinfo.output_scanline < cinfo.output_height)
                (void) jpeg_read_scanlines(&cinfo,
                                           lines + cinfo.output_scanline,
                                           cinfo.output_height);
            (void) jpeg_finish_decompress(&cinfo);
        }

        if ( cinfo.output_components == 3 ) {
            // Expand 24->32 bpp.
            for (uint j=0; j<cinfo.output_height; j++) {
                uchar *in = image.scanLine(j) + cinfo.output_width * 3;
                QRgb *out = (QRgb*)image.scanLine(j);

                for (uint i=cinfo.output_width; i--; ) {
                    in-=3;
                    out[i] = qRgb(in[0], in[1], in[2]);
                }
            }
        }
        iio->setImage(image);
        iio->setStatus(0);
    }

    jpeg_destroy_decompress(&cinfo);
    delete iod_src;
}


struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(QImageIO*);
};


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

    static
    void qt_init_destination(j_compress_ptr)
    {
    }

    static
    void qt_exit_on_error(j_compress_ptr cinfo, QIODevice* dev)
    {
        if (dev->status() == IO_Ok) {
            return;
        } else {
            // cinfo->err->msg_code = JERR_FILE_WRITE;
            (*cinfo->err->error_exit)((j_common_ptr)cinfo);
        }
    }

    static
    boolean qt_empty_output_buffer(j_compress_ptr cinfo)
    {
        my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
        QIODevice* dev = dest->iio->ioDevice();

        if ( dev->writeBlock( (char*)dest->buffer, max_buf ) != max_buf )
            qt_exit_on_error(cinfo, dev);

        dest->next_output_byte = dest->buffer;
        dest->free_in_buffer = max_buf;

#if defined(_OS_UNIXWARE7_)
        return B_TRUE;
#else
        return TRUE;
#endif
    }

    static
    void qt_term_destination(j_compress_ptr cinfo)
    {
        my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
        QIODevice* dev = dest->iio->ioDevice();
        int n = max_buf - dest->free_in_buffer;

        if ( dev->writeBlock( (char*)dest->buffer, n ) != n )
            qt_exit_on_error(cinfo, dev);

        dev->flush();

        qt_exit_on_error(cinfo, dev);
    }

#if defined(Q_C_CALLBACKS)
}
#endif


inline
my_jpeg_destination_mgr::my_jpeg_destination_mgr(QImageIO* iioptr)
{
    jpeg_destination_mgr::init_destination = qt_init_destination;
    jpeg_destination_mgr::empty_output_buffer = qt_empty_output_buffer;
    jpeg_destination_mgr::term_destination = qt_term_destination;
    iio = iioptr;
    next_output_byte = buffer;
    free_in_buffer = max_buf;
}


int qt_jpeg_quality = 75; //### remove 3.0 ?

static
void write_jpeg_image(QImageIO* iio)
{
}

#endif

static bool bInit = false;

__declspec(dllexport) void qInitJpeg()
{
#ifndef QT_NO_IMAGEIO_JPEG
    // Not much to go on - just 3 bytes: 0xFF, M_SOI, 0xFF
    // Even the third is not strictly specified as required.
    if (bInit)
        return;
    bInit = true;
    QImageIO::defineIOHandler("JPEG", "^\377\330\377", 0, read_jpeg_image, write_jpeg_image);
#endif
}

