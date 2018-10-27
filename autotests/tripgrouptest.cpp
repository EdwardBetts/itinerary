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

#include <reservationmanager.h>
#include <tripgroupmanager.h>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TripGroupTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        for (const auto id : mgr->reservations()) {
            mgr->removeReservation(id);
        }
    }

    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testGrouping()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));

        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 0);
        }

        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QEXPECT_FAIL("", "not working yet", Continue);
            QCOMPARE(addSpy.size(), 1);
        }
    }

};
QTEST_GUILESS_MAIN(TripGroupTest)

#include "tripgrouptest.moc"
