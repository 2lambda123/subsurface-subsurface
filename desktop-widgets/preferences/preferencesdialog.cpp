#include "preferencesdialog.h"

#include "abstractpreferenceswidget.h"
#include "preferences_language.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QDebug>

PreferencesDialogV2::PreferencesDialogV2()
{
	pagesList = new QListWidget();
	pagesStack = new QStackedWidget();
	buttonBox = new QDialogButtonBox(
		QDialogButtonBox::Save |
		QDialogButtonBox::RestoreDefaults |
		QDialogButtonBox::Cancel);

	pagesList->setMinimumWidth(120);
	pagesList->setMaximumWidth(120);

	QHBoxLayout *h = new QHBoxLayout();
	h->addWidget(pagesList);
	h->addWidget(pagesStack);
	QVBoxLayout *v = new QVBoxLayout();
	v->addLayout(h);
	v->addWidget(buttonBox);

	setLayout(v);

	addPreferencePage(new PreferencesLanguage());
	refreshPages();

	connect(pagesList, &QListWidget::currentRowChanged,
		pagesStack, &QStackedWidget::setCurrentIndex);
	connect(buttonBox, &QDialogButtonBox::clicked,
		this, &PreferencesDialogV2::buttonClicked);
}

PreferencesDialogV2::~PreferencesDialogV2()
{
}

void PreferencesDialogV2::buttonClicked(QAbstractButton* btn)
{
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(btn);
	switch(role) {
		case QDialogButtonBox::AcceptRole : applyRequested(); return;
		case QDialogButtonBox::RejectRole : cancelRequested(); return;
		case QDialogButtonBox::ResetRole : defaultsRequested(); return;
	}
}

bool abstractpreferenceswidget_lessthan(AbstractPreferencesWidget *p1, AbstractPreferencesWidget *p2)
{
	return p1->positionHeight() <= p2->positionHeight();
}

void PreferencesDialogV2::addPreferencePage(AbstractPreferencesWidget *page)
{
	pages.push_back(page);
	qSort(pages.begin(), pages.end(), abstractpreferenceswidget_lessthan);
}

void PreferencesDialogV2::refreshPages()
{
	// Remove things
	pagesList->clear();
	while(pagesStack->count()) {
		QWidget *curr = pagesStack->widget(0);
		pagesStack->removeWidget(curr);
		curr->setParent(0);
	}

	// Readd things.
	Q_FOREACH(AbstractPreferencesWidget *page, pages) {
		QListWidgetItem *item = new QListWidgetItem(page->icon(), page->name());
		pagesList->addItem(item);
		pagesStack->addWidget(page);
	}
}

void PreferencesDialogV2::applyRequested()
{
	qDebug() << "Apply Clicked";
}

void PreferencesDialogV2::cancelRequested()
{
	qDebug() << "Cancel Clicked";
}

void PreferencesDialogV2::defaultsRequested()
{
	qDebug() << "Defaults Clicked";
}
