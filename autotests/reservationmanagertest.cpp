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

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class ReservationManagerTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        for (const auto id : mgr->reservations()) {
            mgr->removeReservation(id);
        }
    }

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testOperations()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

        QSignalSpy addSpy(&mgr, &ReservationManager::reservationAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &ReservationManager::reservationUpdated);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &ReservationManager::reservationRemoved);
        QVERIFY(rmSpy.isValid());

        QVERIFY(mgr.reservations().isEmpty());
        mgr.importReservation(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json"));

        auto res = mgr.reservations();
        QCOMPARE(res.size(), 1);
        const auto resId = res.at(0);
        QVERIFY(!resId.isEmpty());

        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), resId);
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(!mgr.reservation(resId).isNull());

        mgr.importReservation(QLatin1String(SOURCE_DIR "/data/4U8465-v2.json"));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.reservations().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.reservation(resId).isValid());

        mgr.removeReservation(resId);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.reservations().isEmpty());
        QVERIFY(mgr.reservation(resId).isNull());
    }
};
QTEST_GUILESS_MAIN(ReservationManagerTest)

#include "reservationmanagertest.moc"