/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "applicationcontroller.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Place>

#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>

using namespace KItinerary;

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent)
{
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::showOnMap(const QVariant &place)
{
    if (place.isNull()) {
        return;
    }

    // TODO Android implementation

    const auto geo = JsonLdDocument::readProperty(place, "geo").value<GeoCoordinates>();
    if (geo.isValid()) {
        // zoom out further from airports, they are larger and you usually want to go further away from them
        const auto zoom = place.userType() == qMetaTypeId<Airport>() ? 12 : 17;
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/"));
        const QString fragment = QLatin1String("map=") + QString::number(zoom)
                                    + QLatin1Char('/') + QString::number(geo.latitude())
                                    + QLatin1Char('/') + QString::number(geo.longitude());
        url.setFragment(fragment);
        QDesktopServices::openUrl(url);
        return;
    }

    const auto addr = JsonLdDocument::readProperty(place, "address").value<PostalAddress>();
    if (!addr.isEmpty()) {
        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("www.openstreetmap.org"));
        url.setPath(QStringLiteral("/search"));
        const QString queryString = addr.streetAddress() + QLatin1String(", ")
                                    + addr.postalCode() + QLatin1Char(' ')
                                    + addr.addressLocality() + QLatin1String(", ")
                                    + addr.addressCountry();
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("query"), queryString);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    }
}
