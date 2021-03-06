#include "application.h"
#include "columnartablewidget.h"
#include "columnselectdialog.h"
#include "QsLog.h"
#include "exportsearchdialog.h"
#include "centeredcheckbox.h"
#include "lanesupport.h"
#include "externs.h"
//#include "focustable.h"
ColumnarTableWidget::ColumnarTableWidget(const QStringList & headers,QWidget * parent) : QTableWidget(parent) {
  m_settings = 0;
  m_saveConfig = true;
  m_columnHeadings = headers;
  m_defaultWidthKey = "Default width";
  m_columnWidthsKey = "Column widths";
  m_stateKey = "Column state";
  m_defaultWidth = -1;
  m_markColumn = -1;
  setRowCount(0);
  setColumnCount(m_columnHeadings.size());
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setHorizontalHeaderLabels(m_columnHeadings);
  horizontalHeader()->setStretchLastSection(true);
  horizontalHeader()->setSectionsMovable(true);
  horizontalHeader()->setSectionsClickable(true);
  verticalHeader()->setVisible(false);
  verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
  //verticalHeader()->setDefaultSectionSize(44);
  connect(this->horizontalHeader(),SIGNAL(sectionDoubleClicked(int)),this,SLOT(onColumnDialog(int)));
}
ColumnarTableWidget::~ColumnarTableWidget() {
  if (m_settings != 0) {
    writeConfiguration();
    delete m_settings;
  }
}
void ColumnarTableWidget::setFixedRowHeight(int h) {
    verticalHeader()->setDefaultSectionSize(h);
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
  ColumnSelectDialog d(m_columnHeadings);
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
    if (c[i]) {     // if its visible
      if (this->columnWidth(i) == 0) {  // zero width, it was invisible
        this->setColumnWidth(i,m_defaultWidth);
      }
    }
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
 * The passed QSettings is cloned, so the calling routine can do what it wants
 *
 * @param settings a QSettings pointing at the correct group
 */
void ColumnarTableWidget::readConfiguration(QSettings & settings) {

  m_settings = new QSettings(settings.fileName(),settings.format());
  m_settings->setIniCodec(settings.iniCodec());
  m_settings->beginGroup(settings.group());
  if (m_defaultWidth == -1) {
    m_defaultWidth = 200;
  }
  m_defaultWidth = settings.value(m_defaultWidthKey,m_defaultWidth).toInt();
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
        if (w == -1) {
          w = m_defaultWidth;
        }
        sections << w;
      }
    }
    if (sections.size() == m_columnHeadings.size()) {
      for(int i=0;i < sections.size();i++) {
        this->setColumnWidth(i,sections[i]);
      }
    }
  }
  if (m_markColumn != -1) {
    this->resizeColumnToContents(m_markColumn);
  }
}
void ColumnarTableWidget::setDefaultWidth(int v) {
  m_defaultWidth = v;
}

void ColumnarTableWidget::writeConfiguration() {
  QLOG_DEBUG() << Q_FUNC_INFO <<m_saveConfig;
  if (m_saveConfig) {
    m_settings->setValue(m_stateKey,this->horizontalHeader()->saveState());
    m_settings->sync();
  }
}
void ColumnarTableWidget::setSaveConfiguration(bool v) {
  m_saveConfig = v;
}
QLabel * ColumnarTableWidget::createLabel(const QString & text,const QString & style) const {
  //
  /// remove line breaks and leading/trailing spaces
  QString str = text.trimmed();
  str = str.replace(QRegularExpression("\\r|\\n")," ");

  QLabel * l;
  if (! style.isEmpty()) {
    l = new QLabel(getSupport()->scanAndStyle(str,style));
  }
  else {
    l = new QLabel(str);
  }
  return l;
}
/*
bool ColumnarTableWidget::startsWithArabic(const QString & t) const {
  for(int i=0;i < t.size();i++) {
    if (t.at(i).direction() == QChar::DirAL) {
      return true;
    }
    if (t.at(i).direction() == QChar::DirL) {
      return false;
    }

  }
  return false;
}
*/
void ColumnarTableWidget::showEmpty(const QString & text) {
  this->setRowCount(0);
  this->insertRow(0);
  this->setHorizontalHeaderItem(0,new QTableWidgetItem());
  for(int i=1;i < this->columnCount();i++) {
    this->hideColumn(i);
  }
  m_emptyText = text;
  QLabel * label = this->createLabel(text);
  label->setAlignment(Qt::AlignCenter);
  this->setCellWidget(0,0,label);
  m_saveConfig = false;
}
QStringList ColumnarTableWidget::columnHeadings() const {
  QStringList columns;

  for(int i=0;i < this->columnCount();i++) {
    QString s = this->horizontalHeaderItem(i)->text();
    if (i != m_markColumn) {
      if (! s.isEmpty()) {
        columns << s;
      }
    }
  }
  return columns;
}
/**
 * This is not used by textsearchwidget
 *
 * @param key
 *
 * @return
 */
QString ColumnarTableWidget::exportResults(const QString & key) const {
  QStringList columns;

  for(int i=0;i < this->columnCount();i++) {
    QString s = this->horizontalHeaderItem(i)->text();
    if (i != m_markColumn) {
      if (! s.isEmpty()) {
        columns << s;
      }
    }
  }
  ExportSearchDialog dlg(columns,key);
  if (dlg.exec() != QDialog::Accepted) {
    return QString();
  }
  if (dlg.saveSettings()) {
    dlg.writeSettings();
  }
  QString exportFileName = dlg.exportFileName();
  QFile file(exportFileName);
  if (! file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QString err = QString(tr("Error opening export file to %1 : %2")).arg(exportFileName).arg(file.errorString());
    QLOG_WARN() << err;
    QMessageBox::warning(0, tr("Export Search Results"),err);
    return QString();
  }
  if (exportFileName.isEmpty()) {
    return QString();
  }

  QString sep = dlg.separator();
  columns = dlg.columns();
  QTextStream out(&file);
  out.setCodec("UTF-8");

  out << columns.join(sep) << endl;
  /// columns will contain a list of column headings to export
  /// turn this into a list of column numbers
  QList<int> cols;
  for(int i=0;i < this->columnCount();i++) {
    QString s = this->horizontalHeaderItem(i)->text();
    if (! s.isEmpty() && columns.contains(s)) {
        cols << i;
    }
  }
  bool allRows = dlg.allRows();
  int rowCount = this->rowCount();
  bool ok;
  int writeCount = 0;
  QString str;
  if (m_markColumn == -1) {
    allRows = true;
  }
  QRegularExpression rx("[\u2000-\u202e]");
  rx.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);
  for(int i=0;i < rowCount;i++) {
    ok = true;
    if (! allRows ) {
      CenteredCheckBox * m = qobject_cast<CenteredCheckBox *>(this->cellWidget(i,m_markColumn));
      if (m) {
        ok =  m->isChecked();
      }
    }
    if (ok) {
      for(int j=0;j < cols.size();j++) {
        str = this->textForColumn(i,cols[j]);
        str.remove(rx);
        if (m_ignoreText.contains(cols[j])) {
          if (str == m_ignoreText.value(cols[j])) {
            str.clear();
          }
        }
        out <<  str << sep;
      }
      out << endl;
      writeCount++;
    }
  }
  file.close();
  return QString(tr("Exported search results to file: %1 (%2 lines)")).arg(exportFileName).arg(writeCount);;
}
QString ColumnarTableWidget::textForColumn(int row,int col) const {
  QTableWidgetItem * item = this->item(row,col);
  QString str;
  if (item) {
    str = item->text();
  }
  else {
    QLabel * label = qobject_cast<QLabel *>(this->cellWidget(row,col));
    if (label) {
      str = label->text();
    }
  }
  return removeSpan(str);
}
QString ColumnarTableWidget::removeSpan(const QString & str) const {
  QString t = str;
  t.remove(QRegularExpression("<span[^>]+>"));
  t.remove(QRegularExpression("</span>"));
  return t;
}
void ColumnarTableWidget::setMarkColumn(int col) {
  m_markColumn = col;
}
void ColumnarTableWidget::setColumnWidth(int col,int width) {
  m_columnWidths.insert(col,width);
}
void ColumnarTableWidget::setExportIgnore(int col,const QString & text) {
  m_ignoreText.insert(col,text);
}
