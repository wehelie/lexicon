#ifndef __NODEVIEW_H__
#define __NODEVIEW_H__
#include <QDialog>
#include <QString>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QRegExp>
#include "textsearch.h"
class NodeView : public QDialog {
  Q_OBJECT

 public:
  NodeView(QWidget * parent = 0);
  NodeView(const SearchParams &,QWidget * parent = 0);
  ~NodeView();
  void setCSS(const QString &);
  void setHtml(const QString &);
  void setHeader(const QString & root,const QString & head,const QString & node,int page = 0);
  void setPattern(const QRegExp & rx);
  void setStartPosition(int);
  QTextDocument * document() { return m_browser->document(); }
  QSize sizeHint() const;
  public slots:
    void accept();
    void reject();
    void findFirst();
    void findNext();
    void print();
    void openEntry();
 private:
    void setup();
    SearchParams m_params;
    void getPositions();
    QTextCursor m_cursor;
    QSize m_size;
    void setPreferredSize(const QString &);
    QPushButton * m_findFirstButton;
    QPushButton * m_findNextButton;
    //    QPushButton * m_printButton;
    //    QPushButton * m_tabOpenButton;
    QTextBrowser * m_browser;
    QLabel * m_rlabel;
    QLabel * m_hlabel;
    QLabel * m_pageLabel;
    QString m_root;
    QString m_head;
    QString m_node;
    int m_startPosition;
    QRegExp m_pattern;
     QString m_css;
    QList<int> m_positions;
    int m_positionIndex;
 signals:
    void openNode(const QString & node);
    void printNode(const QString & node);
};
#endif
