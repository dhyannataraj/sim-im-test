/*
 * avatarbar.h
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#ifndef AVATARBAR_H_
#define AVATARBAR_H_

#include <QWidget>

class AvatarBar: public QWidget
{
public:
    AvatarBar(QWidget* parent = 0);
    virtual ~AvatarBar();

    void setSourceContactImage(const QImage& image);
    QImage sourceContactImage() const;

    void setTargetContactImage(const QImage& image);
    QImage targetContactImage() const;

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    QImage m_sourceImage;
    QImage m_targetImage;
};

#endif /* AVATARBAR_H_ */
