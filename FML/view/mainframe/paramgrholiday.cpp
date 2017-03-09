#include "paramgrholiday.h"
#include "util/datatype.h"
#include "controller/qcontrollermanager.h"
#include <QStandardItemModel>
#include "util/util.h"
#include "view/commonui/message_box_widget.h"


ParaMgrHoliday::ParaMgrHoliday(QWidget *parent)
	: QWidget(parent)
	, m_pGoodsModel(NULL)
{
	ui.setupUi(this);
	ui.pushButton_add->setText(tr("add"));
	ui.pushButton_add->setIcon(QIcon(qutil::skin("addclickover.png")));
	ui.pushButton_modify->setText(tr("modify"));
	ui.pushButton_modify->setIcon(QIcon(qutil::skin("reviseclickover.png")));
	ui.pushButton_delete->setText(tr("delete"));
	ui.pushButton_delete->setIcon(QIcon(qutil::skin("deleteclickover.png")));
	connect(ui.pushButton_add, &QPushButton::clicked, this, &ParaMgrHoliday::addHoliday);
	connect(ui.pushButton_modify, &QPushButton::clicked, this, &ParaMgrHoliday::modifyHoliday);
	connect(ui.pushButton_delete, &QPushButton::clicked, this, &ParaMgrHoliday::delHoliday);
	init();
}

ParaMgrHoliday::~ParaMgrHoliday()
{
}

void ParaMgrHoliday::init()
{
	{
		ui.treeView->setAlternatingRowColors(true);
		initDateView();
		connect(ui.treeView, &QTreeView::clicked, [this](const QModelIndex &index) {
			QVariant s = index.sibling(index.row(),0).data();
			if (s.isValid())
			{
				ui.dateEdit->setDate(QDate::fromString(s.toString(),"yyyy-MM-dd"));
				QStringList slist = s.toString().split("-");
				if (slist.length() > 0)
					ui.lineEdit_year->setText(slist[0]);
				QVariant sDayType = index.sibling(index.row(), 1).data();
				if (sDayType.isValid())
					ui.comboBox->setCurrentText(sDayType.toString());
				QVariant sDayinfo = index.sibling(index.row(), 2).data();
				if (sDayinfo.isValid())
					ui.textEdit_def->setText(sDayinfo.toString());
			}
		});
	}

	{
		ui.comboBox->addItems(QStringList() << tr("holiday") << tr("workday"));
		ui.comboBox->view()->setAlternatingRowColors(true);
	}
	connect(ui.dateEdit, &QDateTimeEdit::dateChanged, [this](const QDate &date) {
		ui.lineEdit_year->setText(QString::number(date.year()));
	});
}

void ParaMgrHoliday::addHoliday()
{
	qint64 _d = ui.dateEdit->date().toJulianDay(); // ��λ1Ϊ һ��
	QString strDayType = ui.comboBox->currentText();
	QString strDayInfo = ui.textEdit_def->toPlainText();
	int _y = ui.dateEdit->date().year();
	CFinancialCalendar::EHType e = CFinancialCalendar::eWorkDay;
	if (tr("holiday") == strDayType) e = CFinancialCalendar::eHoliday;
	CFinancialCalendar fc(_y, _d, e, strDayInfo);
	if (PARASETCTL->isExistFinancialCalendar(fc))
		ShowWarnMessage(tr("add"), tr("the time is existing."), this);
	else
	{
		if (PARASETCTL->setFinancialCalendar(fc))
		{
			ShowSuccessMessage(tr("add"), tr("add success."), this);
			// ͬ��
			initDateView();
		}
		else
			ShowWarnMessage(tr("add"), tr("add fail."), this);
	}
}
void ParaMgrHoliday::modifyHoliday()
{
	qint64 _d = ui.dateEdit->date().toJulianDay(); // ��λ1Ϊ һ��
	QString strDayType = ui.comboBox->currentText();
	QString strDayInfo = ui.textEdit_def->toPlainText();
	int _y = ui.dateEdit->date().year();
	CFinancialCalendar::EHType e = CFinancialCalendar::eWorkDay;
	if (tr("holiday") == strDayType) e = CFinancialCalendar::eHoliday;
	CFinancialCalendar fc(_y, _d, e, strDayInfo);
	if (!PARASETCTL->isExistFinancialCalendar(fc))
		ShowWarnMessage(tr("modify"), tr("the time does not exist."), this);
	else
	{
		if (PARASETCTL->setFinancialCalendar(fc))
		{
			ShowSuccessMessage(tr("modify"), tr("modify success."), this);
			// ͬ��
			initDateView();
		}
		else
			ShowWarnMessage(tr("modify"), tr("modify fail."), this);
	}
}
void ParaMgrHoliday::delHoliday()
{
	if (MessageBoxWidget::Yes == ShowQuestionMessage(tr("delete"), tr("confirm to delete."), this))
	{
		qint64 _d = ui.dateEdit->date().toJulianDay(); // ��λ1Ϊ һ��
		QString strDayType = ui.comboBox->currentText();
		QString strDayInfo = ui.textEdit_def->toPlainText();
		int _y = ui.dateEdit->date().year();
		CFinancialCalendar::EHType e = CFinancialCalendar::eWorkDay;
		if (tr("holiday") == strDayType) e = CFinancialCalendar::eHoliday;
		CFinancialCalendar fc(_y, _d, e, strDayInfo);
		if (PARASETCTL->isExistFinancialCalendar(fc))
		{
			if (!PARASETCTL->removeFinancialCalendar(_d))
			{
				ShowWarnMessage(tr("delete"), tr("delete fail."), this);
				return;
			}
		}
		else
		{
			ShowWarnMessage(tr("delete"), tr("no day dist."), this);
			return;
		}
		ShowSuccessMessage(tr("delete"), tr("delete success."), this);
		// ͬ��
		initDateView();
	}
}

void ParaMgrHoliday::initDateView()
{
	if (m_pGoodsModel) m_pGoodsModel->clear();
	QStringList treeHeader;
	treeHeader << tr("hyear") << tr("holiday") << tr("holidayinfo");
	if (!m_pGoodsModel) m_pGoodsModel = new QStandardItemModel(0, treeHeader.size(), this);
	m_pGoodsModel->setColumnCount(treeHeader.size());
	for (int i = 0; i < treeHeader.size(); i++)
		m_pGoodsModel->setHeaderData(i, Qt::Horizontal, treeHeader[i]);
	ui.treeView->setModel(m_pGoodsModel);
	QMap<int, CFinancialCalendar> val;
	PARASETCTL->getFinancialCalendar(val);
	QList<QStandardItem *> items;
	for (QMap<int, CFinancialCalendar>::const_iterator itor = val.begin();
		itor != val.end(); itor++)
	{
		if (items.isEmpty() ||
			items.back()->text().toInt() != itor->getYear())
		{
			items << new QStandardItem(QString::number(itor->getYear()));
			m_pGoodsModel->appendRow(items.back());
		}

		QList<QStandardItem *> childItems;
		QStandardItem *itemDate = new QStandardItem(QDate::fromJulianDay(itor->getDate()).toString("yyyy-MM-dd"));
		QStandardItem *itemHolidayType = new QStandardItem(itor->getHolidayTypeStr());
		QStandardItem *itemHolidayinfo = new QStandardItem(itor->getHolidayinfo());
		childItems.push_back(itemDate);
		childItems.push_back(itemHolidayType);
		childItems.push_back(itemHolidayinfo);
		itemDate->setToolTip(QDate::fromJulianDay(itor->getDate()).toString("yyyy-MM-dd"));
		itemHolidayinfo->setToolTip(qutil::splitTooltip(itor->getHolidayinfo(),200));
		items.back()->appendRow(childItems);
	}
}