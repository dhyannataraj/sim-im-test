/*
 * avatarbar.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include "avatarbar.h"
#include <QPainter>

AvatarBar::AvatarBar(QWidget* parent) : QWidget(parent)
{
}

AvatarBar::~AvatarBar()
{
}

void AvatarBar::setSourceContactImage(const QImage& image)
{
    m_sourceImage = image;
}

QImage AvatarBar::sourceContactImage() const
{
    return m_sourceImage;
}

void AvatarBar::setTargetContactImage(const QImage& image)
{
    m_targetImage = image;
}

QImage AvatarBar::targetContactImage() const
{
    return m_targetImage;
}

QSize AvatarBar::sizeHint() const
{
    int maxWidth = std::max(m_sourceImage.width(), m_targetImage.width());
    int totalHeight = m_sourceImage.height() + m_targetImage.height();
    return QSize(maxWidth, totalHeight);
}

void AvatarBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QPoint target((width() - m_targetImage.width()) / 2, 0);
    QPoint source((width() - m_targetImage.width()) / 2, height() - m_sourceImage.height());
    p.drawImage(target, m_targetImage);
    p.drawImage(source, m_sourceImage);
}



