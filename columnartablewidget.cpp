#include "columnartablewidget.h"
#include "columnselectdialog.h"
ColumnarTableWidget::ColumnarTableWidget(const QStringList & headers,QWidget * parent) : QTableWidget(parent) {
  m_settings = 0;
  m_saveConfig = true;
  m_colHeadings = headers;
  m_defaultWidthKey = "Default width";
  m_columnWidthsKey = "Column widths";
  m_stateKey = "State";

  setRowCount(0);
  setColumnCount(m_colHeadings.size());
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setHorizontalHeaderLabels(m_colHeadings);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionsMovable(true);
  horizontalHeader()->setSectionsClickable(true);

  connect(this->horizontalHeader(),SIGNAL(sectionDoubleClicked(int)),this,SLOT(onColumnDialog(int)));
}
ColumnarTableWidget::~ColumnarTableWidget() {
  if (m_settings != 0) {
    writeConfiguration();
    delete m_settings;
  }
}
void ColumnarTableWidget::resetTable() {
  setRowCount(0);
  setColumnCount(m_colHeadings.size());
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setHorizontalHeaderLabels(m_colHeadings);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionsMovable(true);
  horizontalHeader()->setSectionsClickable(true);
}
void ColumnarTableWidget::setKey(int key,const QString & value) {
  switch(key) {
  case DEFAULT_WIDTH : { m_defaultWidthKey = value;break; }
  case COLUMN_WIDTHS : { m_columnWidthsKey = value;break; }
  case STATE : { m_stateKey = value;break; }
  default : break;
  }
}
void ColumnarTableWidget::onColumnDialog(int /* section */) {
  QStringList m;
  ColumnSelectDialog d(m_colHeadings);
  QList<bool> c;
  for(int i=0;i < this->columnCount();i++) {
    c << ! this->horizontalHeader()->isSectionHidden(i);
  }
  d.setState(c);
  if (d.exec() != QDialog::Accepted) {
    return;
  }
  c = d.state();
  for(int i=0;i < c.size();i++) {
    this->horizontalHeader()->setSectionHidden(i,!c[i]);
  }
}
/**
 *
 * read/write saves the column width and order
 * set a default column width
 * if we have saved state, use it
 * otherwise if we have 'columns' set the sizes from that, provided there are as many entries as
 * there are columns
 *
 * @param settings a QSettings pointing at the correct group
 */
void ColumnarTableWidget::readConfiguration(QSettings & settings) {
  m_settings = new QSettings(settings.fileName(),settings.format());
  m_settings->setIniCodec(settings.iniCodec());
  m_settings->beginGroup(settings.group());
  m_defaultWidth = settings.value(m_defaultWidthKey,200).toInt();
  this->horizontalHeader()->setDefaultSectionSize(m_defaultWidth);
  QByteArray b = settings.value(m_stateKey,QByteArray()).toByteArray();
  if (b.size() > 0) {
    this->horizontalHeader()->restoreState(b);
  }
  else {
    bool ok;
    int w;
    QList<int> sections;
    QStringList v = settings.value(m_columnWidthsKey).toStringList();
    for(int i=0;i < v.size();i++) {
      w = v[i].toInt(&ok);
      if (ok) {
        sections << w;
      }
    }
    if (sections.size() == m_colHeadings.size()) {
      for(int i=0;i < sections.size();i++) {
        this->setColumnWidth(i,sections[i]);
      }
    }
  }
}
void ColumnarTableWidget::writeConfiguration() {
  if (m_saveConfig) {
    m_settings->setValue(m_stateKey,this->horizontalHeader()->saveState());
  }
}
void ColumnarTableWidget::setSaveConfiguration(bool v) {
  m_saveConfig = v;
}