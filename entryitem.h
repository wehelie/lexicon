#ifndef __ENTRYITEM_H__
#define __ENTRYITEM_H__
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextStream>
#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QSplitter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QApplication>
#include <QTextOption>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QSettings>
#include <QRegExp>
#include <QClipboard>
#include "QsLog.h"
#include "xsltsupport.h"
#include "history.h"
#include "place.h"
#include "notedialog.h"
#include "notes.h"
class ToolButtonData : public QToolButton {
  Q_OBJECT
 public:
  ToolButtonData(int id);
  int getIndex() { return m_id;}
 private:
  int m_id;
};
class EntryItem : public QGraphicsTextItem {
  Q_OBJECT

 public:
  EntryItem(const QString &, QGraphicsItem * parent = 0);
  EntryItem(QGraphicsItem * parent = 0);
  ~EntryItem();
  void setNode(const QString & id);
  void setRoot(const QString & root,bool isRootEntry = false);
  void setWord(const QString & word);
  void setSupplement(int v);
  void setPage(int x);
  void setPlace(const Place &);
  void setBackground(QColor & c) { m_backgroundColor = c;}
  void selectAll();
  void setNotes(QList<Note *> notes);
  bool hasNotes() const;
  Place getPlace();
  QString getNode() { return m_place.getNode();}
  QString getRoot() { return m_place.getRoot();}
  QString getWord() { return m_place.getWord();}
  bool isRoot() { return m_place.isRoot();}
  int getSupplement() { return m_place.getSupplement();}
  int getPage() { return m_place.getPage();}
  QString getOutputHTML() const { return m_html; }
  void setOutputHTML(const QString & html) { m_html = html;}
  QTextCursor highlight(const QString &,Qt::GlobalColor color = Qt::yellow);
  QTextCursor highlightRx(const QString &);
  void setFocusOnHover(bool v) { m_focusOnHover = v;};
  bool  getFocusOnHover() const { return m_focusOnHover;}
  ///  return -1 for no note else id key of note
  int  getNoteId();
  int findCount() const { return m_searchPositions.size(); }
  void showNote();
  void addNote();
  void clearHighlights();
  public slots:
    void searchItem();

    void clearSelection();
 signals:
    void showPerseus(const Place &);
    void placeChanged(const Place &);
    void bookmarkAdd(const QString & id,const Place & p);
    void selectAllItems();
    void clearAllItems();
    void copy();
    void gotoNode(const Place & p, bool createTab = true);
    void addButton();
 protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
  void focusInEvent(QFocusEvent *);
  void paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);


  QColor m_backgroundColor;
  bool m_focusOnHover;
 private:
  NoteDialog * m_note;
  QList<Note *> m_notes;
  QList<int> m_searchPositions;
  Place m_place;
  QString m_searchText;
  QString m_html;    /// saves the generated HTML for debug, only set when dumpOutputHTM is true
};
#endif
