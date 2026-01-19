/*******************************************************************************
 * retroshare-gui/src/gui/WikiPoos/WikiUserNotify.cpp                         *
 *                                                                             *
 * Copyright 2014-2024 Retroshare Team <retroshare.project@gmail.com>          *
 *                                                                             *
 * This program is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU Affero General Public License as              *
 * published by the Free Software Foundation, either version 3 of the          *
 * License, or (at your option) any later version.                             *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU Affero General Public License for more details.                         *
 *                                                                             *
 * You should have received a copy of the GNU Affero General Public License    *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.       *
 *                                                                             *
 *******************************************************************************/

#include "retroshare/rswiki.h"
#include "WikiUserNotify.h"
#include "gui/MainWindow.h"
#include "gui/common/FilesDefs.h"

WikiUserNotify::WikiUserNotify(RsGxsIfaceHelper *ifaceImpl, const GxsGroupFrameDialog *g, QObject *parent) :
    GxsUserNotify(ifaceImpl, g, parent)
{
	// Don't count child messages for Wiki (only count new pages/snapshots)
	mCountChildMsgs = false;
}

bool WikiUserNotify::hasSetting(QString *name, QString *group)
{
	if (name) *name = tr("Wiki Page");
	if (group) *group = "Wiki";

	return true;
}

QIcon WikiUserNotify::getIcon()
{
	return FilesDefs::getIconFromQtResourcePath(":/icons/png/wiki.png");
}

QIcon WikiUserNotify::getMainIcon(bool hasNew)
{
    return hasNew ? FilesDefs::getIconFromQtResourcePath(":/icons/png/wiki-notify.png") : FilesDefs::getIconFromQtResourcePath(":/icons/png/wiki.png");
}

void WikiUserNotify::iconClicked()
{
	MainWindow::showWindow(MainWindow::Wiki);
}
