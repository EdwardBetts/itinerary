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

#include "modelverificationpoint.h"

#include <countryinformation.h>
#include <pkpassmanager.h>
#include <reservationmanager.h>
#include <timelinemodel.h>
#include <tripgroupmanager.h>

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/Reservation>

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
#include <QAbstractItemModelTester>
#endif
#include <QDirIterator>
#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TimelineModelTest : public QObject
{
    Q_OBJECT
private:
    void clearPasses(PkPassManager *mgr)
    {
        for (const auto &id : mgr->passes()) {
            mgr->removePass(id);
        }
    }

    void clearReservations(ReservationManager *mgr)
    {
        for (const auto &id : mgr->reservations()) {
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
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testModel()
    {
        PkPassManager mgr;
        clearPasses(&mgr);
        ReservationManager resMgr;
        clearReservations(&resMgr);

        resMgr.setPkPassManager(&mgr);
        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setReservationManager(&resMgr);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QVERIFY(updateSpy.isEmpty());
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);

        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 0);
        QCOMPARE(model.rowCount(), 2);

        clearReservations(&resMgr);
        QCOMPARE(insertSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(model.rowCount(), 1);
    }

    void testNestedElements()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);

        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/haus-randa-v1.json")));
        QCOMPARE(insertSpy.size(), 3);
        QCOMPARE(insertSpy.at(0).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(0).at(2).toInt(), 0);
        QCOMPARE(insertSpy.at(1).at(1).toInt(), 1);
        QCOMPARE(insertSpy.at(1).at(2).toInt(), 1);
        QVERIFY(updateSpy.isEmpty());
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Hotel);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementRangeRole), TimelineModel::RangeBegin);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Hotel);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementRangeRole), TimelineModel::RangeEnd);

        // move end date of a hotel booking: dataChanged on RangeBegin, move (or del/ins) on RangeEnd
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/haus-randa-v2.json")));
        QCOMPARE(insertSpy.size(), 4);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toModelIndex().row(), 1);
        QCOMPARE(insertSpy.at(2).at(1).toInt(), 0);
        QCOMPARE(insertSpy.at(2).at(2).toInt(), 0);
        QCOMPARE(rmSpy.at(0).at(1), 2);
        QCOMPARE(model.rowCount(), 4);

        // delete a split element
        const auto resId = model.data(model.index(1, 0), TimelineModel::ReservationIdsRole).toStringList().value(0);
        QVERIFY(!resId.isEmpty());
        resMgr.removeReservation(resId);
        QCOMPARE(rmSpy.size(), 4);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
    }

    void testCountryInfos()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);

        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setReservationManager(&resMgr);

        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/flight-txl-lhr-sfo.json")));
        QCOMPARE(model.rowCount(), 5); //  2x country info, 2x flights, today marker

        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        auto countryInfo = model.index(0, 0).data(TimelineModel::CountryInformationRole).value<CountryInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Left);
        QCOMPARE(countryInfo.drivingSideDiffers(), true);
        QCOMPARE(countryInfo.powerPlugCompatibility(), CountryInformation::Incompatible);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);

        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        countryInfo = model.index(2, 0).data(TimelineModel::CountryInformationRole).value<CountryInformation>();
        QCOMPARE(countryInfo.drivingSide(), KItinerary::KnowledgeDb::DrivingSide::Right);
        QCOMPARE(countryInfo.drivingSideDiffers(), false);
        QCOMPARE(countryInfo.powerPlugCompatibility(), CountryInformation::Incompatible);
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        // remove the GB flight should also remove the GB country info
        auto resId = model.index(1, 0).data(TimelineModel::ReservationIdsRole).toStringList().value(0);
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::CountryInfo);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);

        // remove the US flight should also remove the US country info
        resId = model.index(1, 0).data(TimelineModel::ReservationIdsRole).toStringList().value(0);
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
    }

    void testWeatherElements()
    {
        using namespace KItinerary;

        ReservationManager resMgr;
        clearReservations(&resMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setReservationManager(&resMgr);
        model.setWeatherForecastManager(&weatherMgr);
        QCOMPARE(model.rowCount(), 1); // no weather data, as we don't know where we are

        // Add an element that will result in a defined location
        GeoCoordinates geo;
        geo.setLatitude(52.0f);
        geo.setLongitude(13.0f);
        Airport a;
        a.setGeo(geo);
        Flight f;
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(QDate(2018, 1, 1), QTime(0, 0)));
        FlightReservation res;
        res.setReservationFor(f);
        resMgr.addReservation(res);

        QCOMPARE(model.rowCount(), 11); // 1x flight, 1x today, 9x weather
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        auto fc = model.index(2, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate::currentDate());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(model.index(10, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        fc = model.index(10, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate::currentDate().addDays(8));

        // Add a flight one day from now changing location mid-day
        geo.setLatitude(46.0f);
        geo.setLongitude(8.0f);
        a.setGeo(geo);
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(QDate::currentDate().addDays(1), QTime(12, 0)));
        f.setArrivalTime(QDateTime(QDate::currentDate().addDays(1), QTime(14, 0)));
        res.setReservationFor(f);
        resMgr.addReservation(res);

        QCOMPARE(model.rowCount(), 13); // 2x flight, 1x today, 10x weather
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        fc = model.index(2, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime().date(), QDate::currentDate());
        QCOMPARE(model.index(3, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(1), QTime(0, 0)));
        QCOMPARE(model.index(4, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(5, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        fc = model.index(5, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(1), QTime(14, 0)));
        QCOMPARE(model.index(6, 0).data(TimelineModel::ElementTypeRole), TimelineModel::WeatherForecast);
        fc = model.index(6, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QVERIFY(fc.isValid());
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(2), QTime(0, 0)));

        // check we get update signals for all weather elements
        QSignalSpy spy(&model, &TimelineModel::dataChanged);
        QVERIFY(spy.isValid());
        emit weatherMgr.forecastUpdated();
        QCOMPARE(spy.size(), 10);

        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(1), QTime(0, 0)));
        fc = model.index(9, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(5), QTime(0, 0)));

        // add a location change far in the future, this must not change anything
        geo.setLatitude(60.0f);
        geo.setLongitude(11.0f);
        a.setGeo(geo);
        f.setArrivalAirport(a);
        f.setDepartureTime(QDateTime(QDate::currentDate().addYears(1), QTime(6, 0)));
        res.setReservationFor(f);
        resMgr.addReservation(res);
        QCOMPARE(model.rowCount(), 14);

        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(1), QTime(0, 0)));
        fc = model.index(9, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(5), QTime(0, 0)));

        // result is the same when data hasn't been added incrementally
        model.setReservationManager(nullptr);
        model.setReservationManager(&resMgr);
        QCOMPARE(model.rowCount(), 14);

        fc = model.index(3, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 13.0f);
        QCOMPARE(fc.maximumTemperature(), 52.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(1), QTime(0, 0)));
        fc = model.index(9, 0).data(TimelineModel::WeatherForecastRole).value<WeatherForecast>();
        QVERIFY(fc.isValid());
        QCOMPARE(fc.minimumTemperature(), 8.0f);
        QCOMPARE(fc.maximumTemperature(), 46.0f);
        QCOMPARE(fc.dateTime(), QDateTime(QDate::currentDate().addDays(5), QTime(0, 0)));

        // clean up
        auto resId = model.index(13, 0).data(TimelineModel::ReservationIdsRole).toStringList().value(0);
        resMgr.removeReservation(resId);
        resId = model.index(4, 0).data(TimelineModel::ReservationIdsRole).toStringList().value(0);
        resMgr.removeReservation(resId);
        QCOMPARE(model.rowCount(), 11);

        // test case: two conesequtive location changes, the first one to an unknown location
        // result: the weather element before the first location change ends with the start of that
        // result 2: we get a second weather element the same day after the second location change
        // TODO
    }

    void testMultiTraveller()
    {
        using namespace KItinerary;

        ReservationManager resMgr;
        clearReservations(&resMgr);
        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setReservationManager(&resMgr);
        QCOMPARE(model.rowCount(), 1); // 1x TodayMarker

        QSignalSpy insertSpy(&model, &TimelineModel::rowsInserted);
        QVERIFY(insertSpy.isValid());
        QSignalSpy updateSpy(&model, &TimelineModel::dataChanged);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&model, &TimelineModel::rowsRemoved);
        QVERIFY(rmSpy.isValid());

        // full import at runtime
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(model.rowCount(), 3); // 2x Flight, 1x TodayMarger
        QCOMPARE(insertSpy.count(), 2);
        QCOMPARE(updateSpy.count(), 2);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ReservationIdsRole).toStringList().size(), 2);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ReservationIdsRole).toStringList().size(), 2);

        // already existing data
        model.setReservationManager(nullptr);
        model.setReservationManager(&resMgr);
        QCOMPARE(model.rowCount(), 3); // 2x Flight, 1x TodayMarger
        QCOMPARE(model.index(0, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ElementTypeRole), TimelineModel::Flight);
        QCOMPARE(model.index(2, 0).data(TimelineModel::ElementTypeRole), TimelineModel::TodayMarker);
        QCOMPARE(model.index(0, 0).data(TimelineModel::ReservationIdsRole).toStringList().size(), 2);
        QCOMPARE(model.index(1, 0).data(TimelineModel::ReservationIdsRole).toStringList().size(), 2);

        // update splits element
        updateSpy.clear();
        insertSpy.clear();
        auto resId = model.index(1, 0).data(TimelineModel::ReservationIdsRole).toStringList().value(0);
        QVERIFY(!resId.isEmpty());
        auto res = resMgr.reservation(resId).value<FlightReservation>();
        auto flight = res.reservationFor().value<Flight>();
        flight.setDepartureTime(flight.departureTime().addDays(1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        QCOMPARE(model.rowCount(), 4);
        QCOMPARE(updateSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 1);
        QCOMPARE(rmSpy.count(), 0);

        // update merges two elements
        updateSpy.clear();
        insertSpy.clear();
        rmSpy.clear();
        flight.setDepartureTime(flight.departureTime().addDays(-1));
        res.setReservationFor(flight);
        resMgr.updateReservation(resId, res);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(updateSpy.count(), 1);
        QCOMPARE(rmSpy.count(), 1);
        QCOMPARE(insertSpy.count(), 0);

        // removal of merged items
        updateSpy.clear();
        rmSpy.clear();
        clearReservations(&resMgr);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(rmSpy.count(), 2);
        QCOMPARE(updateSpy.count(), 2);
    }

    void testDayChange()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        WeatherForecastManager weatherMgr;
        weatherMgr.setTestModeEnabled(true);

        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2196, 10, 14}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setWeatherForecastManager(&weatherMgr);

        ModelVerificationPoint vp0(QLatin1String(SOURCE_DIR "/data/timeline/daychange-r0.model"));
        vp0.setRoleFilter({TimelineModel::ReservationIdsRole});
        QVERIFY(vp0.verify(&model));

        // changing the day should move the today marker
        model.setCurrentDateTime(QDateTime({2196, 10, 15}, {0, 15}));
        ModelVerificationPoint vp1(QLatin1String(SOURCE_DIR "/data/timeline/daychange-r1.model"));
        vp1.setRoleFilter({TimelineModel::ReservationIdsRole});
        QVERIFY(vp1.verify(&model));

        // load something to define the current location, so we get weather
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/flight-txl-lhr-sfo.json")));
        ModelVerificationPoint vp2(QLatin1String(SOURCE_DIR "/data/timeline/daychange-r2.model"));
        vp2.setRoleFilter({TimelineModel::ReservationIdsRole});
        QVERIFY(vp2.verify(&model));

        // changing the day should move the today marker and weather one day forward
        model.setCurrentDateTime(QDateTime({2196, 10, 16}, {19, 30}));
        ModelVerificationPoint vp3(QLatin1String(SOURCE_DIR "/data/timeline/daychange-r3.model"));
        vp3.setRoleFilter({TimelineModel::ReservationIdsRole});
        QVERIFY(vp3.verify(&model));
    }

    void testContent_data()
    {
        QTest::addColumn<QString>("baseName");

        QDirIterator it(QLatin1String(SOURCE_DIR "/data/timeline/"), {QLatin1String("*.json")});
        while (it.hasNext()) {
            it.next();
            const auto baseName = it.fileInfo().baseName();
            QTest::newRow(baseName.toUtf8().constData()) << baseName;
        }
    }

    void testContent()
    {
        QFETCH(QString, baseName);
        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/timeline/") + baseName + QLatin1String(".json")));
        TripGroupManager groupMgr;
        groupMgr.setReservationManager(&resMgr);

        TimelineModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&model);
#endif
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({1996, 10, 14}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTripGroupManager(&groupMgr);

        // check state is correct for data imported at the start
        ModelVerificationPoint vp(QLatin1String(SOURCE_DIR "/data/timeline/") + baseName + QLatin1String(".model"));
        vp.setRoleFilter({TimelineModel::ReservationIdsRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp.verify(&model));

        // retry with loading during runtime
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/timeline/") + baseName + QLatin1String(".json")));
        QVERIFY(vp.verify(&model));
    }
};

QTEST_GUILESS_MAIN(TimelineModelTest)

#include "timelinemodeltest.moc"
