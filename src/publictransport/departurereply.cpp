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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "departurereply.h"
#include "departurerequest.h"
#include "logging.h"

#include "backends/navitiaclient.h"
#include "backends/navitiaparser.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/Location>

#include <QNetworkReply>

using namespace KPublicTransport;

namespace KPublicTransport {
class DepartureReplyPrivate {
public:
    std::vector<Departure> departures;
    QString errorMsg;
    DepartureReply::Error error = DepartureReply::NoError;
};
}

DepartureReply::DepartureReply(const DepartureRequest &req, QNetworkAccessManager *nam)
    : d(new DepartureReplyPrivate)
{
    // TODO
}

DepartureReply::~DepartureReply() = default;

std::vector<Departure> DepartureReply::departures() const
{
    return d->departures; // TODO this copies
}

DepartureReply::Error DepartureReply::error() const
{
    return d->error;
}

QString DepartureReply::errorString() const
{
    return d->errorMsg;
}
