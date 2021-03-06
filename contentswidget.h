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
#include <QSettings>
#include "QsLog.h"
#include "place.h"
#include "application.h"
#define ROOT_COLUMN 0
class ContentsWidget : public QTreeWidget {
  Q_OBJECT;
 public:
  ContentsWidget(QWidget * parent = 0);
  ~ContentsWidget();
  void loadContents();
  QString findNextRoot(const QString &);
  QString findPrevRoot(const QString &);
  Place getCurrentPlace();
  Place findNextPlace(const Place &);
  Place findPrevPlace(const Place &);
  QTreeWidgetItem * findPlace(const Place &) const;
  void ensurePlaceVisible(const Place & p, bool select = false);
  int addEntries(const QString & root,QTreeWidgetItem *);
  void reloadFont();
  public slots:
    void nodeExpanded(QTreeWidgetItem * item);
    void nodeCollapsed(QTreeWidgetItem * item);
 protected:
  void focusInEvent(QFocusEvent *);
  void focusOutEvent(QFocusEvent *);
  void mouseMoveEvent(QMouseEvent *);
  void mousePressEvent(QMouseEvent *);
  void contextMenuEvent(QContextMenuEvent *);
 private:
  QString itypeText(const QString &);
  QStringList m_itypesText;
  void toggleExpand();
  void readSettings();
  bool m_showHeadWord;
  bool m_showEntryWord;
  bool m_showNode;
  bool m_showSupplement;
  bool m_romanItypes;
  /// this is the background color of the select item when the window
  /// does not have focus
  QString m_backgroundColor;
  QSqlQuery m_entryQuery;
  QString m_moveUp;
  QString m_moveDown;
  QString m_expand;
  QString m_dragIconFileName;
  QFont m_itypeFont;
  QPoint m_startPos;
 protected:
  virtual void 	keyPressEvent(QKeyEvent * event);
 signals:
  //  void itemActivated(QTreeWidgetItem *,int /* not used */);
  void atEnd();
  void atStart();
};
#endif
