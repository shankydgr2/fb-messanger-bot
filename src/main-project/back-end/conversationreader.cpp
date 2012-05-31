/*
 * Copyright (C) 2012 Andres Pagliano, Gabriel Miretti, Gonzalo Buteler,
 * Nestor Bustamante, Pablo Perez de Angelis
 *
 * This file is part of LVK Botmaster.
 *
 * LVK Botmaster is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LVK Botmaster is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LVK Botmaster.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "conversationreader.h"
#include "csvrow.h"
#include "csvdocument.h"
#include "globalstrings.h"

#include <QIODevice>
#include <QFile>

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

namespace
{

void makeEntry(Lvk::BE::Conversation::Entry & entry, const Lvk::Common::CsvRow & row)
{
    entry.clear();

    if (row.size() == 6) {
        entry.dateTime = QDateTime::fromString(row[0], STR_CSV_CONV_DATE_TIME_FORMAT);
        entry.from     = row[1];
        entry.to       = row[2];
        entry.msg      = row[3];
        entry.response = row[4];
        entry.match    = row[5] == STR_CSV_CONV_ENTRY_MATCH ? true : false;
    }
}

//--------------------------------------------------------------------------------------------------

void makeConversation(Lvk::BE::Conversation & conv, const Lvk::Common::CsvDocument & doc)
{
    conv.clear();

    Lvk::BE::Conversation::Entry entry;

    foreach (const Lvk::Common::CsvRow &row, doc.rows()) {
        makeEntry(entry, row);
        if (!entry.isNull()) {
            conv.append(entry);
        }
    }
}

} // namespace

//--------------------------------------------------------------------------------------------------
// ConversationReader
//--------------------------------------------------------------------------------------------------

Lvk::BE::ConversationReader::ConversationReader()
    : m_device(0)
{
}

//--------------------------------------------------------------------------------------------------

Lvk::BE::ConversationReader::ConversationReader(const QString &filename)
    : m_device(new QFile(filename))
{
    m_device->open(QIODevice::ReadOnly);
}

//--------------------------------------------------------------------------------------------------

Lvk::BE::ConversationReader::ConversationReader(QIODevice *device)
    : m_device(device)
{
    if (!m_device->isOpen()) {
        m_device->open(QIODevice::ReadOnly);
    }
}

//--------------------------------------------------------------------------------------------------

Lvk::BE::ConversationReader::~ConversationReader()
{
    delete m_device;
}

//--------------------------------------------------------------------------------------------------

bool Lvk::BE::ConversationReader::read(Lvk::BE::Conversation *conv)
{
    if (!m_device) {
        return false;
    }

    // TODO use readLine()
    QString data = QString::fromUtf8(m_device->readAll().constData());

    Common::CsvDocument doc(data);
    makeConversation(*conv, doc);

    return true;
}

//--------------------------------------------------------------------------------------------------

bool Lvk::BE::ConversationReader::read(Conversation::Entry *entry)
{
    if (!m_device) {
        return false;
    }

    char buf[10*1024];
    qint64 len = m_device->readLine(buf, sizeof(buf));

    if (len != -1) {
        Common::CsvRow row(QString::fromUtf8(buf));
        makeEntry(*entry, row);
    }

    return len != -1;
}

//--------------------------------------------------------------------------------------------------

bool Lvk::BE::ConversationReader::atEnd()
{
    return !m_device || m_device->atEnd();
}
