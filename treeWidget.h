/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2018 <tsujan2000@gmail.com>
 *
 * Arqiver is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arqiver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <https://spdx.org/licenses/GPL-3.0+.html>
 */

#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPointer>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QIcon>
#include <QApplication>
#include <QTimer>

namespace Arqiver {

class TreeWidget : public QTreeWidget
{
  Q_OBJECT

public:
  TreeWidget(QWidget *parent = nullptr) : QTreeWidget(parent) {
    timer_ = nullptr;
    dragStarted_ = false; // not needed
    setDragDropMode(QAbstractItemView::DragOnly);
    setContextMenuPolicy (Qt::CustomContextMenu);
  }

  QTreeWidgetItem *getItemFromIndex(const QModelIndex &index) const {
    return itemFromIndex(index);
  }

  QModelIndex getIndexFromItem(const QTreeWidgetItem *item) const {
    return indexFromItem(item);
  }

signals:
  void dragStarted(QTreeWidgetItem *it);
  void enterPressed(QTreeWidgetItem *it);

protected:
  virtual void mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
      if (indexAt(event->pos()).isValid())
        dragStartPosition_ = event->pos();
    }
    dragStarted_ = false;
  }

  virtual void mouseMoveEvent(QMouseEvent *event) {
    if (!dragStartPosition_.isNull()
        && (event->pos() - dragStartPosition_).manhattanLength() >= qMax(22, QApplication::startDragDistance()))
    {
      dragStarted_ = true;
    }

    if ((event->buttons() & Qt::LeftButton) && dragStarted_)
    {
      //dragStarted_ = false;
      window()->setAcceptDrops(false);
      QTreeWidgetItem *it = currentItem();
      if (!it || it->text(1).isEmpty()) { // it's a directory item
        event->accept();
        return;
      }
      emit dragStarted(it);

      QPointer<QDrag> drag = new QDrag(this);
      QMimeData *mimeData = new QMimeData;
      mimeData->setData("application/arqiver-item", QByteArray());
      mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(it->data(0, Qt::UserRole).toString()));
      drag->setMimeData(mimeData);
      QPixmap px = it->icon(0).pixmap (22, 22);
      drag->setPixmap(px);
      drag->setHotSpot(QPoint(px.width()/2, px.height()));
      if (drag->exec (Qt::CopyAction) == Qt::IgnoreAction)
        drag->deleteLater();
      event->accept();
      window()->setAcceptDrops(true);
    }
  }

  virtual void keyReleaseEvent(QKeyEvent *event) {
    /* NOTE: If Enter is kept pressed, it will be released and then will be
             pressed again, and so on. As a workaround, a timer is used here. */
    if (currentItem() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
      if(!timer_) {
        timer_ = new QTimer (this);
        timer_->setSingleShot(true);
        timer_->start(250);
      }
      else {
        if (timer_->isActive()) {
          event->accept();
          return;
        }
        timer_->start(250);
      }
      emit enterPressed(currentItem());
      event->accept();
      return;
    }
    QTreeWidget::keyReleaseEvent(event);
  }

private:
  QPoint dragStartPosition_;
  bool dragStarted_;
  QTimer *timer_;
};

}

#endif // TREEWIDGET_H
