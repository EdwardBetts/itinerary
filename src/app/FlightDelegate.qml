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

import QtQuick 2.5
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1 as QQC2
import org.kde.kirigami 2.0 as Kirigami
import "." as App

App.TimelineDelegate {
    id: root
    implicitHeight: topLayout.implicitHeight

    ColumnLayout {
        id: topLayout
        width: root.width

        Rectangle {
            id: headerBackground
            Layout.fillWidth: true
            color: Kirigami.Theme.complementaryBackgroundColor
            implicitHeight: headerLayout.implicitHeight

            RowLayout {
                id: headerLayout
                anchors.left: parent.left
                anchors.right: parent.right

                QQC2.Label {
                    text: qsTr("✈ %1 %2 %3 -> %4")
                        .arg(reservation.reservationFor.airline.iataCode)
                        .arg(reservation.reservationFor.flightNumber)
                        .arg(reservation.reservationFor.departureAirport.iataCode)
                        .arg(reservation.reservationFor.arrivalAirport.iataCode)
                    color: Kirigami.Theme.complementaryTextColor
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: reservation.reservationFor.boardingTimeLocalized
                    color: Kirigami.Theme.complementaryTextColor
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
                }
            }
        }

        QQC2.Label {
            text: qsTr("Terminal: %1  Gate: %2  Seat: %3")
                .arg(reservation.reservationFor.departureTerminal)
                .arg(reservation.reservationFor.departureGate)
                .arg(reservation.airplaneSeat)
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: qsTr("Departure from %1: %2")
                .arg(reservation.reservationFor.departureAirport.name)
                .arg(reservation.reservationFor.departureTimeLocalized)
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: qsTr("Arrival at %1: %2")
                .arg(reservation.reservationFor.arrivalAirport.name)
                .arg(reservation.reservationFor.arrivalTimeLocalized)
            color: Kirigami.Theme.textColor
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Boarding Pass")
            onClicked: showBoardingPass(root.pass, root.passId)
            visible: root.passId !== ""
            icon.source: root.passId !== "" ? "image://org.kde.pkpass/" + passId + "/icon" : ""
        }

    }
}