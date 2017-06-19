#ifndef __TEXTSEARCHWIDGET_H__
#define __TEXTSEARCHWIDGET_H__
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
class TextSearch;
class ColumnarTableWidget;
class Place;
class TextSearchWidget : public QWidget {
  Q_OBJECT

 public:
  TextSearchWidget(int pageSize = 10,bool summary = false,QWidget * parent = 0);
  void load(const TextSearch &);
  void loadPage(int page);
  void setPages(int);
  TextSearch * searcher();
  void readSettings();
 private:
  TextSearch * m_data;
  bool m_summary;
  QString m_contextStyle;
  ColumnarTableWidget * m_results;
  int addRow(const Place & p, const QString & text,int pos);
  int m_pageSize;
  QComboBox * m_page;
  bool m_resizeRows;
  int m_rowHeight;
  int m_stepCount;
  private slots:
    void pageChanged(const QString &);

};
#endif