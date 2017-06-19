#include "textsearchwidget.h"
#include "textsearch.h"
#include "lanesupport.h"
#include "columnartablewidget.h"
#include "textsearch.h"
#include "centeredcheckbox.h"
#include "place.h"
#include "definedsettings.h"
#include "externs.h"
#define SELECT_COLUMN 0
#define ROOT_COLUMN 1
#define HEAD_COLUMN 2
#define NODE_COLUMN 3
#define POSITION_COLUMN 4
#define VOL_COLUMN 5
#define CONTEXT_COLUMN 6
extern LaneSupport * getSupport();
TextSearchWidget::TextSearchWidget(int pageSize,bool summary,QWidget * parent) : QWidget(parent) {
  QStringList headings;
  headings << tr("Mark") << tr("Root") << tr("Headword") << tr(" Node ") << tr("Occurs")  << tr("Vol/Page") << tr("Context");
  if (! summary) {
    headings[POSITION_COLUMN] = tr("Position");
  }
  readSettings();
  m_summary = summary;
  m_pageSize = pageSize;
  m_results = new ColumnarTableWidget(headings,this);
  m_results->setFixedRowHeight(m_rowHeight);
  m_results->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_results->setSelectionMode(QAbstractItemView::SingleSelection);
  m_results->setMarkColumn(SELECT_COLUMN);
  QVBoxLayout * layout = new QVBoxLayout;
  layout->addWidget(m_results);
  m_page = new QComboBox(this);
  connect(m_page,SIGNAL(currentIndexChanged(const QString &)),this,SLOT(pageChanged(const QString &)));
  QHBoxLayout * hlayout = new QHBoxLayout;
  hlayout->addWidget(new QLabel(tr("Page")));
  hlayout->addWidget(m_page);
  hlayout->addStretch();
  layout->addLayout(hlayout);
  m_results->setRowCount(pageSize);
  setLayout(layout);
  m_data = new TextSearch();
  m_data->setXsltFileName(getSupport()->xsltFileName());
}
TextSearch * TextSearchWidget::searcher() {
  return m_data;
}
void TextSearchWidget::setPages(int pages) {
  m_page->blockSignals(true);
  m_page->clear();
  for(int i=0;i < pages;i++) {
    m_page->addItem(QString("%1").arg(i+1));
  }
  if (pages > 0) {
    m_page->setCurrentIndex(0);
  }
  m_page->blockSignals(false);
}
void TextSearchWidget::load(const TextSearch & data) {
  int rows = data.rows(m_summary);
  QList<SearchHit> d = data.getHits(0,rows,m_summary);
  for(int i=0;i < d.size();i++) {
    Place p = Place::fromSearchHit(d[i]);
    addRow(p,d[i].fragment,d[i].ix);
  }
}
void TextSearchWidget::loadPage(int page) {
  m_results->setRowCount(0);
  //  int rows = data.rows(m_summary);
  QList<SearchHit> d = m_data->getPage(page,m_summary);
  for(int i=0;i < d.size();i++) {
    Place p = Place::fromSearchHit(d[i]);
    addRow(p,d[i].fragment,d[i].ix);
  }
  m_results->resizeColumnToContents(SELECT_COLUMN);
  m_results->resizeColumnToContents(NODE_COLUMN);
  m_results->resizeColumnToContents(POSITION_COLUMN);
  m_results->resizeColumnToContents(CONTEXT_COLUMN);
  if (m_resizeRows) {
    m_results->resizeRowsToContents();
  }
}
void TextSearchWidget::pageChanged(const QString & page) {
  bool ok;
  int p = page.toInt(&ok,10);
  if (ok) {
    this->loadPage(p);
  }
}
int TextSearchWidget::addRow(const Place & p, const QString & text,int pos) {

  QTableWidgetItem * item;
  QLabel * label;

  int row = m_results->rowCount();
  m_results->insertRow(row);

  m_results->setCellWidget(row,SELECT_COLUMN,new CenteredCheckBox);

  label = new QLabel(getSupport()->scanAndStyle(p.root(),"fullsearchlist"));
  label->setAlignment(Qt::AlignCenter);
  m_results->setCellWidget(row,ROOT_COLUMN,label);

  label = m_results->createLabel(p.head(),"fullsearchlist");
  label->setAlignment(Qt::AlignCenter);
  m_results->setCellWidget(row,HEAD_COLUMN,label);

  label = m_results->createLabel(p.node());
  label->setAlignment(Qt::AlignCenter);
  m_results->setCellWidget(row,NODE_COLUMN,label);

  label = m_results->createLabel((QString("%1").arg(pos)));
  label->setAlignment(Qt::AlignCenter);
  m_results->setCellWidget(row,POSITION_COLUMN,label);

  label = m_results->createLabel(text,"fullsearchlist");

  if (getSupport()->startsWithArabic(text)) {
    label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  }
  else {
    label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  }

  if ( ! m_contextStyle.isEmpty()) {
    QString str = label->text();
    str = QString("<span style=\"%1\">%2</span>").arg(m_contextStyle).arg(str);
    label->setText(str);
  }

  m_results->setCellWidget(row,CONTEXT_COLUMN,label);
  label = m_results->createLabel(p.format("%V/%P"));
  label->setAlignment(Qt::AlignCenter);
  m_results->setCellWidget(row,VOL_COLUMN,label);

  return row;
}
void TextSearchWidget::readSettings() {
  SETTINGS

  settings.beginGroup("FullSearch");
  m_contextStyle = settings.value(SID_FULLSEARCH_CONTEXT_STYLE,QString()).toString();
  //  m_singleRow = settings.value(SID_FULLSEARCH_ONE_ROW,true).toBool();
  QString f = settings.value(SID_FULLSEARCH_RESULTS_FONT,QString()).toString();
  if (! f.isEmpty()) {
    //    m_resultsFont.fromString(f);
  }
  m_resizeRows = settings.value(SID_FULLSEARCH_RESIZE_ROWS,true).toBool();
  m_rowHeight  = settings.value(SID_FULLSEARCH_ROW_HEIGHT,40).toInt();;

}
