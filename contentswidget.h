#ifndef __CONTENTSWIDGET_H__
#define __CONTENTSWIDGET_H__
#include <QTreeWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QHeaderView>
#include <QDebug>
#include <QFont>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include "QsLog.h"
class ContentsWidget : public QTreeWidget {
  Q_OBJECT;
 public:
  ContentsWidget(QWidget * parent = 0);
  void loadContents();
  QString findNextRoot(const QString &);

 private:
  bool m_debug;
 protected:
  virtual void 	keyPressEvent(QKeyEvent * event);
 signals:
  void itemActivated(QTreeWidgetItem *,int /* not used */);
};
#endif
