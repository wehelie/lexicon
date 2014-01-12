#include "contentswidget.h"

ContentsWidget::ContentsWidget(QWidget * parent) : QTreeWidget(parent) {
  m_debug = true;
  setColumnCount(2);
  setHeaderLabels(
                  QStringList() << tr("Contents") << tr("Supplement"));
  setSelectionMode(QAbstractItemView::SingleSelection);
  header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
}
void ContentsWidget::loadContents() {
  QSqlQuery query;
  QFile f;
  QTextStream * out;
  if (m_debug) {
    QFileInfo fi(QDir::tempPath(),QString("roots.txt"));
    f.setFileName(fi.filePath());
    if (f.open(QIODevice::WriteOnly)) {
      out = new QTextStream(&f);
      out->setCodec("UTF-8");
    }
  }
  QString sql = "select distinct letter from root order by letter";
  if ( ! query.prepare(sql)) {
    QLOG_FATAL() << "Error preparing SQL:" << sql;
    return;
  }
  query.exec();

  QSqlQuery rootQuery;
  sql = "select word,supplement from root where letter = ? order by word,supplement";
  if (! rootQuery.prepare(sql)) {
    QLOG_FATAL() << "Error preparing SQL:" << sql;
    return;
  }

  while(query.next()) {
    QString letter = query.value(0).toString();
    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(letter));
    rootQuery.bindValue(0,letter);
    rootQuery.exec();
    //    if (! rootQuery.first()) {
    //      QLOG_DEBUG() << rootQuery.lastError().text();
    //    }
    QString supp;
    QString root;
    while(rootQuery.next()) {
      bool ok = true;
      root = rootQuery.value(0).toString();
      //      itype = rootQuery.value(2).toString();

      if (rootQuery.value(1).toInt() == 1) {
        supp = "*";
        //        if (itype == "alphabetical letter") {
          //          ok = false;
        //        }

      }
      else {
        supp = " ";
      }
      /// skipping the letter entries from the supplement
      if ((supp == "*") && (root.size() == 1)) {
        ok = false;
      }
      if (ok) {
        if (m_debug && f.isOpen()) {
          *out << root << '\n';
        }
        QStringList cols;
        cols << root << supp;
        QTreeWidgetItem * rootitem = new QTreeWidgetItem(item,cols);
      }
    }
    addTopLevelItem(item);
  }
  if (m_debug) {
    out->flush();
    f.close();
    delete out;
  }
}
/**
 * Called by LanesLexicon when it gets a next root signal
 *
 *
 * @param root start root
 *
 * @return the next root in sequence
 */
QString ContentsWidget::findNextRoot(const QString & root) {
  QTreeWidgetItem * currentItem;
  int tc = topLevelItemCount();
  int topIndex = -1;
  int childIndex = -1;
  bool found = false;

  for(int i = 0;(i < tc) && ! found;i++) {
    QTreeWidgetItem * topItem = topLevelItem(i);
    int kidCount = topItem->childCount();
    for(int j=0;(j < kidCount) && ! found ;j++) {
      QTreeWidgetItem * child = topItem->child(j);
      if (child->text(0) == root) {
        currentItem = child;
        topIndex = i;
        if (j == (kidCount - 1)) {
          topIndex++;
          childIndex = 0;
        }
        else {
          childIndex = j + 1;
        }
        found = true;
      }
    }
  }
  if (topIndex == tc) {
    emit(atEnd());
    return QString();
  }
  /// overkill, but would only matter if there were letters without any roots
  for(int i = topIndex;i < tc; i++) {
      QTreeWidgetItem * item = topLevelItem(i);
      int kidCount = item->childCount();
      if (childIndex < kidCount) {
        QTreeWidgetItem * nextItem = item->child(childIndex);
        currentItem->setSelected(false);
        nextItem->setSelected(true);
        return nextItem->text(0);
      }
    }
  return QString();
}
/**
 * Called by LanesLexicon when it gets a next root signal
 *
 *
 * @param root start root
 *
 * @return the next root in sequence
 */
QString ContentsWidget::findPrevRoot(const QString & root) {
  QTreeWidgetItem * currentItem;
  int tc = topLevelItemCount();
  int topIndex = -1;
  int childIndex = -1;
  bool found = false;

  for(int i = 0;(i < tc) && ! found;i++) {
    QTreeWidgetItem * topItem = topLevelItem(i);
    int kidCount = topItem->childCount();
    for(int j=0;(j < kidCount) && ! found ;j++) {
      QTreeWidgetItem * child = topItem->child(j);
      if (child->text(0) == root) {
        currentItem = child;
        /// if first child, we want the last root of the prev letter
        topIndex = i;
        if (j == 0) {
          topIndex--;
          childIndex = -1;
        }
        else {
          childIndex = j - 1;
        }
        found = true;
      }
    }
  }
  if (topIndex == -1) {
    emit(atStart());
    return QString();
  }
  /// overkill, but would only matter if there were letters without any roots
  for(int i = topIndex;i >= 0; i--) {
      QTreeWidgetItem * item = topLevelItem(i);
      int kidCount = item->childCount();
      if (kidCount > childIndex) {
        if (childIndex == -1) {
          childIndex = kidCount - 1;
        }
        QTreeWidgetItem * nextItem = item->child(childIndex);
        currentItem->setSelected(false);
        nextItem->setSelected(true);
        return nextItem->text(0);
      }
    }
  return QString();
}
void ContentsWidget::keyPressEvent(QKeyEvent * event) {
  switch (event->key()) {
  case Qt::Key_Space: {
    QTreeWidgetItem * item = this->currentItem();
    if (item) {
      /// it is a top level item so expand it
      if (item->childCount() > 0) {
        if (item->isExpanded()) {
          collapseItem(item);
        }
        else {
          expandItem(item);
        }
      }
      else {
        emit(itemActivated(item,0));
      }
    }
    break;
  }
  default:
    QTreeWidget::keyPressEvent(event);
  }
}
void ContentsWidget::ensureVisible(const QString & root, bool select) {
  QTreeWidgetItem * item;
  QTreeWidgetItem * topItem;
  int tc = topLevelItemCount();
  int topIndex = -1;
  int childIndex = -1;
  bool found = false;

  for(int i = 0;(i < tc) && ! found;i++) {
    topItem = topLevelItem(i);
    int kidCount = topItem->childCount();
    for(int j=0;(j < kidCount) && ! found ;j++) {
      item = topItem->child(j);
      if (item->text(0) == root) {
        topIndex = i;
        childIndex = j;
        found = true;
      }
    }
  }
  if (!item) {
    return;
  }
  topItem->setExpanded(true);
  qDebug() << "hidden" << item->isHidden() << "top" << topItem->isExpanded();
  if (select) {
    setCurrentItem(item);
  }
  QModelIndex index = indexFromItem(item);
  scrollTo(index);
}
