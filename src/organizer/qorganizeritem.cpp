/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qorganizeritem.h"

#include <QSet>
#include <QDebug>
#include <QDataStream>

#include "qorganizeritem.h"
#include "qorganizeritem_p.h"
#include "qorganizeritemdetail_p.h"
#include "qorganizermanager_p.h"

#include "qorganizeritemdetails.h"

QTORGANIZER_BEGIN_NAMESPACE

/*!
  \macro Q_DECLARE_CUSTOM_ORGANIZER_ITEM
  \relates QOrganizerItem

  Macro for simplifying declaring convenience leaf classes for QOrganizerItem.

  The first argument is the name of the class, and the second argument
  is a Latin-1 string literal naming the item type.

  If you are creating a convenience class for a type of QOrganizerItem,
  you should use this macro when declaring your class to ensure that
  it interoperates with other organizer item functionality.
 */

/*!
  \class QOrganizerItem

  \brief The QOrganizerItem class represents an event, todo, note, or journal entry

  \inmodule QtOrganizer
  \since 1.1

  \ingroup organizer-main

  A QOrganizerItem object has an id and a collection of details (like a start date and location), as
  well as a collection id which identifies which QOrganizerCollection the item is part of in a manager.
  Each detail (which can have multiple fields) is stored in an appropriate subclass of QOrganizerItemDetail,
  and the QOrganizerItem allows retrieving these details in various ways. QOrganizerItemCustomDetail is
  supposed to be used to store user specific details that are not pre-defined in the detal leaf classes.

  Most clients will want to use the convenient subclasses of QOrganizerItem (i.e., QOrganizerEvent
  (and QOrganizerEventOccurence), QOrganizerTodo (and QOrganizerTodoOccurence), QOrganizerJournal and
  QOrganizerNote) instead of manipulating instances of QOrganizerItem directly.

  A QOrganizerItem instance represents the in-memory version of a calendar organizer item.
  It is possible for the contents of a QOrganizerItem to change independently of the contents
  that are stored persistently in a QOrganizerManager.  A QOrganizerItem has an id associated
  with it when it is first retrieved from a QOrganizerManager, or after it has been first saved,
  and this allows clients to track changes using the signals in QOrganizerManager.  When saved
  in a manager, every item is placed into a QOrganizerCollection in that manager, according
  to the collection id set in the item prior to save (or the default collection if no
  collection id was set in the item).

  Different QOrganizerManagers may require an item to have certain details saved in it before
  it can be stored in that manager.  By default, every item must have a QOrganizerItemType
  detail which identifies the type of the item.  Different subclasses of QOrganizerItem
  (i.e., QOrganizerEvent (and QOrganizerEventOccurence), QOrganizerTodo (and QOrganizerTodoOccurence),
  QOrganizerJournal and QOrganizerNote) may have other mandatory details, depending on the manager.

  \sa QOrganizerManager, QOrganizerItemDetail
 */

/*!
 * \fn QList<T> QOrganizerItem::details() const
 * Returns a list of details of the template parameter type.  The type must be
 * a subclass of QOrganizerItemDetail.
   \since 1.1
 */

/*!
 * \fn T QOrganizerItem::detail() const
 * Returns the first detail of the template parameter type, as returned by the template details() function.
 * The type must be a subclass of QOrganizerItemDetail.
   \since 1.1
 */

/*!
 * \fn QOrganizerItem::operator!=(const QOrganizerItem &other) const
 * Returns true if this organizer item's id or details are different to those of the \a other organizer item.
   \since 1.1
 */

/*!
    Construct an empty organizer item.

    The organizer item will have an empty display label, an empty id, and an empty description
    and have type \l QOrganizerItemType::TypeUnknown.
    The isEmpty() function will return true.
   \since 1.1
*/
QOrganizerItem::QOrganizerItem()
    : d(new QOrganizerItemData)
{
    clearDetails();
}

/*! Initializes this QOrganizerItem from \a other
   \since 1.1
*/
QOrganizerItem::QOrganizerItem(const QOrganizerItem& other)
    : d(other.d)
{
}

/*!
    \internal

    Constructs a new, empty item of the given type \a type.
*/
QOrganizerItem::QOrganizerItem(QOrganizerItemType::ItemType type)
{
    d = new QOrganizerItemData;
    clearDetails();
    setType(type);
}

/*!
    \internal

    Constructs an item that is a copy of \a other if \a other is of the expected type
    identified by \a expectedType, else constructs a new, empty item of the
    type identified by the \a expectedType.

    The \a expectedType pointer must be valid for the lifetime of the program.
*/
QOrganizerItem::QOrganizerItem(const QOrganizerItem& other, QOrganizerItemType::ItemType expectedType)
{
    if (other.type() == expectedType) {
        d = other.d;
    } else {
        d = new QOrganizerItemData;
        setType(expectedType);
    }
}

/*!
    \internal

    Assigns this item to \a other if the type of \a other is that identified
    by the given \a expectedType, else assigns this item to be a new, empty
    item of the type identified by the given \a expectedType
    \since 1.1
*/
QOrganizerItem& QOrganizerItem::assign(const QOrganizerItem& other, QOrganizerItemType::ItemType expectedType)
{
    if (this != &other) {
        if (other.type() == expectedType) {
            d = other.d;
        } else {
            d = new QOrganizerItemData;
            setType(expectedType);
        }
    }
    return *this;
}


/*!
 * Returns true if this QOrganizerItem is empty, false if not.
 *
 * An empty QOrganizerItem has an empty label and no extra details.
 * The type of the organizer item is irrelevant.
    \since 1.1
 */
bool QOrganizerItem::isEmpty() const
{
    /* Every organizer item has a type.. */
    if (d->m_details.count() > 1)
        return false;

    /* if there's a label, is it empty? */
    //const QOrganizerItemDisplayLabel& label = detail<QOrganizerItemDisplayLabel>();
    //return label.label().isEmpty();
    /* if there's a description, is it empty? */
    //const QOrganizerItemDescription& description = detail<QOrganizerItemDescription>();
    //return description.description().isEmpty();

    // XXX TODO:
    // depending on the type, different details may be mandatory!
    return true;
}

/*!
    Removes all details of the organizer item.

    This function does not modify the id or type of the organizer item.
    Calling isEmpty() after calling this function will return true.
 */
void QOrganizerItem::clearDetails()
{
    d->m_details.clear();

    // and the organizer item type detail.
    QOrganizerItemType organizeritemType;
    organizeritemType.setType(QOrganizerItemType::TypeUndefined);
    organizeritemType.d->m_access = QOrganizerItemDetail::Irremovable;
    d->m_details.insert(0, organizeritemType);
}

/*! Replace the contents of this QOrganizerItem with \a other
    \since 1.1
*/
QOrganizerItem& QOrganizerItem::operator=(const QOrganizerItem& other)
{
    d = other.d;
    return *this;
}

/*! Frees the memory used by this QOrganizerItem */
QOrganizerItem::~QOrganizerItem()
{
}

/*!
    Returns the QOrganizerItemId that identifies this organizer item.

    This may have been set when the organizer item was retrieved from
    a particular manager, or when the organizer item was first saved
    in a manager.  The QOrganizerItemId is only valid within a specific
    manager.  See \l QOrganizerManager::saveItem() for more
    information.
    \since 1.1
 */
QOrganizerItemId QOrganizerItem::id() const
{
    return d->m_id;
}

/*!
  Returns the id of the collection which this item is part of, in the manager
  in which the item has been saved, if the item has previously been saved in
  a manager.  If the item has not previously been saved in a manager, this function
  will return the id of the collection into which the client wishes the item to be
  saved when \l QOrganizerManager::saveItem() is called, which is set by calling
  \l setId(); otherwise, returns a null id.

  An item always belongs to exactly one collection in a particular manager after it
  has been saved in the manager.  If the item has previously been saved in the manager,
  in a particular collection, and the client sets the collection id of the item to
  the id of a different collection within that manager and then resaves the item,
  the item will be moved from its original collection into the specified collection
  if the move operation is supported by the manager; otherwise, the
  \l QOrganizerManager::saveItem() operation will fail and calling
  \l QOrganizerManager::error() will return \c QOrganizerManager::NotSupportedError.
  \since 1.1
 */
QOrganizerCollectionId QOrganizerItem::collectionId() const
{
    return d->m_collectionId;
}

/*!
  Sets the id of the collection into which the client wishes the item to be saved
  to the given \a collectionId.

  If the given \a collectionId is the null collection id, the client is specifying
  that the item should be saved into the collection in which the item is already
  saved (if the item has previously been saved in the manager, without having been
  removed since), or into the default collection of the manager (if the item has
  not previously been saved in the manager, or has been removed since the last time
  it was saved).

  If the item has previously been saved in a particular manager, and the given
  \a collectionId is the id of a different collection than the one which the
  item is currently a part of in that manager, saving the item with
  \l QOrganizerManager::saveItem() will move the item from its original
  collection to the collection whose id is \a collectionId, if \a collectionId
  identifies a valid collection and the operation is supported by the manager.
  \since 1.1
 */
void QOrganizerItem::setCollectionId(const QOrganizerCollectionId& collectionId)
{
    d->m_collectionId = collectionId;
}

/*!
 * Sets the id of this organizer item to \a id.
 *
 * Note that this only affects this object, not any corresponding structures stored
 * by a QOrganizerManager.
 *
 * If you change the id of a organizer item and save the organizer item
 * in a manager, the previously existing organizer item will still
 * exist.  You can do this to create copies (possibly modified)
 * of an existing organizer item, or to save a organizer item in a different manager.
 *
 * \sa QOrganizerManager::saveItem()
   \since 1.1
 */
void QOrganizerItem::setId(const QOrganizerItemId& id)
{
    d->m_id = id;
    // TODO - reset collection id?
}

/*!
    Returns the first detail stored in the organizer item with the given \a definitionName
    \since 1.1
*/
QOrganizerItemDetail QOrganizerItem::detail(const QString& definitionName) const
{
    if (definitionName.isEmpty())
        return d->m_details.first();

    // build the sub-list of matching details.
    for (int i = 0; i < d->m_details.size(); i++) {
        const QOrganizerItemDetail& existing = d->m_details.at(i);
        if (QOrganizerItemDetailPrivate::detailPrivate(existing)->m_definitionName == definitionName) {
            return existing;
        }
    }

    return QOrganizerItemDetail();
}

/*! Returns a list of details with the given \a definitionName
    The definitionName string can be determined by the DefinitionName attribute
    of defined objects (e.g. QOrganizerItemPhoneNumber::DefinitionName) or by
    requesting a list of all the definition names using
    \l {QOrganizerManager::detailDefinitions()}{detailDefinitions()}.
    \since 1.1
*/
QList<QOrganizerItemDetail> QOrganizerItem::details(const QString& definitionName) const
{
    // build the sub-list of matching details.
    QList<QOrganizerItemDetail> sublist;

    // special case
    if (definitionName.isEmpty()) {
        sublist = d->m_details;
    } else {
        for (int i = 0; i < d->m_details.size(); i++) {
            const QOrganizerItemDetail& existing = d->m_details.at(i);
            if (QOrganizerItemDetailPrivate::detailPrivate(existing)->m_definitionName == definitionName) {
                sublist.append(existing);
            }
        }
    }

    return sublist;
}

/*!
 * Saves the given \a detail in the list of stored details, and sets the detail's id.
 * If another detail of the same type and id has been previously saved in
 * this organizer item, that detail is overwritten.  Otherwise, a new id is generated
 * and set in the detail, and the detail is added to the organizer item.
 *
 * If the detail's access constraint includes \c QOrganizerItemDetail::ReadOnly,
 * this function will return true and save the detail in the organizer item,
 * however attempting to save the organizer item in a manager may fail (if that manager
 * decides that the read only detail should not be updated).
 * Details with the \c QOrganizerItemDetail::ReadOnly constraint set are typically provided
 * in a organizer item by the manager, and are usually information that is either
 * synthesized, or not intended to be changed by the user (e.g. presence information
 * for other organizer items).
 *
 * If \a detail is a QOrganizerItemType, the existing organizer item type will
 * be overwritten with \a detail.  There is never more than one organizer item type
 * in a organizer item.
 *
 * Returns true if the detail was saved successfully, otherwise returns false.
 *
 * Note that the caller retains ownership of the detail.
   \since 1.1
 */
bool QOrganizerItem::saveDetail(QOrganizerItemDetail* detail)
{
    if (!detail)
        return false;

    /* Also handle organizer item type specially - only one of them. */
    if (QOrganizerItemDetailPrivate::detailPrivate(*detail)->m_definitionName == QOrganizerItemType::DefinitionName) {
        for (int i = 0; i < d->m_details.size(); i++) {
            if (detail->d->m_definitionName == d->m_details.at(i).d->m_definitionName) {
                detail->d->m_access = d->m_details.at(i).accessConstraints();
                d->m_details.replace(i, *detail);
                return true;
            }
        }
        // doesn't already exist; append it.
        d->m_details.append(*detail);
        return true;
    }

    /* And description */
    if (QOrganizerItemDetailPrivate::detailPrivate(*detail)->m_definitionName == QOrganizerItemDescription::DefinitionName) {
        for (int i = 0; i < d->m_details.size(); i++) {
            if (detail->d->m_definitionName == d->m_details.at(i).d->m_definitionName) {
                detail->d->m_access = d->m_details.at(i).accessConstraints();
                d->m_details.replace(i, *detail);
                return true;
            }
        }
        // doesn't already exist; append it.
        d->m_details.append(*detail);
        return true;
    }

    /* And display label.. */
    if (QOrganizerItemDetailPrivate::detailPrivate(*detail)->m_definitionName == QOrganizerItemDisplayLabel::DefinitionName) {
        for (int i = 0; i < d->m_details.size(); i++) {
            if (detail->d->m_definitionName == d->m_details.at(i).d->m_definitionName) {
                detail->d->m_access = d->m_details.at(i).accessConstraints();
                d->m_details.replace(i, *detail);
                return true;
            }
        }
        // doesn't already exist; append it.
        d->m_details.append(*detail);
        return true;
    }

    /* And classification.. */
    if (QOrganizerItemDetailPrivate::detailPrivate(*detail)->m_definitionName == QOrganizerItemClassification::DefinitionName) {
        for (int i = 0; i < d->m_details.size(); i++) {
            if (detail->d->m_definitionName == d->m_details.at(i).d->m_definitionName) {
                detail->d->m_access = d->m_details.at(i).accessConstraints();
                d->m_details.replace(i, *detail);
                return true;
            }
        }
        // doesn't already exist; append it.
        d->m_details.append(*detail);
        return true;
    }

    // try to find the "old version" of this field
    // ie, the one with the same type and id, but different value or attributes.
    for (int i = 0; i < d->m_details.size(); i++) {
        const QOrganizerItemDetail& curr = d->m_details.at(i);
        if (detail->d->m_definitionName == curr.d->m_definitionName && detail->d->m_id == curr.d->m_id) {
            // update the detail constraints of the supplied detail
            detail->d->m_access = d->m_details.at(i).accessConstraints();
            // Found the old version.  Replace it with this one.
            d->m_details[i] = *detail;
            return true;
        }
    }
    // this is a new detail!  add it to the organizer item.
    d->m_details.append(*detail);
    return true;
}

/*!
 * Removes the \a detail from the organizer item.
 *
 * The detail in the organizer item which has the same key as that of the given \a detail
 * will be removed if it exists.  Only the key is used for comparison - that is, the
 * information in the detail may be different.
 *
 * If the detail's access constraint includes \c QOrganizerItemDetail::Irremovable,
 * this function will return false.
 *
 * Returns true if the detail was removed successfully, false if an error occurred.
 *
 * Note that the caller retains ownership of the detail.
   \since 1.1
 */
bool QOrganizerItem::removeDetail(QOrganizerItemDetail* detail)
{
    if (!detail)
        return false;

    // find the detail stored in the organizer item which has the same key as the detail argument
    int removeIndex = -1;
    for (int i = 0; i < d->m_details.size(); i++) {
        if (d->m_details.at(i).key() == detail->key()) {
            removeIndex = i;
            break;
        }
    }

    // make sure the detail exists (in some form) in the organizer item.
    if (removeIndex < 0)
        return false;

    if (detail->accessConstraints() & QOrganizerItemDetail::Irremovable)
        return false;

    // Type -detail is specific case which cannot be deleted
    if (QOrganizerItemType::DefinitionName == QOrganizerItemDetailPrivate::detailPrivate(*detail)->m_definitionName)
        return false;

    if (!d->m_details.contains(*detail))
        return false;

    // then remove the detail.
    d->m_details.removeAt(removeIndex);
    return true;
}

/*! Returns true if this organizer item is equal to the \a other organizer item, false if either the id, collection id or stored details are not the same
   \since 1.1
*/
bool QOrganizerItem::operator==(const QOrganizerItem& other) const
{
    if (other.d.constData()->m_id != d.constData()->m_id
        || other.d.constData()->m_collectionId != d.constData()->m_collectionId
        || d.constData()->m_details.size() != other.d.constData()->m_details.size()) {
        return false;
    }

    QList<QOrganizerItemDetail> searchList(d.constData()->m_details);

    foreach (const QOrganizerItemDetail &detail, other.d.constData()->m_details) {
        if (!searchList.removeOne(detail))
            return false;
    }
    return true;
}

/*!
    \relates QOrganizerItem
    Returns the hash value for \a key.
   \since 1.1
*/
uint qHash(const QOrganizerItem &key)
{
    uint hash = qHash(key.id());
    hash += qHash(key.collectionId());
    foreach (const QOrganizerItemDetail& detail, key.details()) {
        hash += qHash(detail);
    }
    return hash;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  Streams the \a organizeritem to the given debug stream \a dbg, and returns the stream.
   \since 1.1
 */
QDebug operator<<(QDebug dbg, const QOrganizerItem& organizeritem)
{
    dbg.nospace() << "QOrganizerItem(" << organizeritem.id() << ") in collection(" << organizeritem.collectionId() << ")";
    foreach (const QOrganizerItemDetail& detail, organizeritem.details()) {
        dbg.space() << '\n' << detail;
    }
    return dbg.maybeSpace();
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
 * Writes \a item to the stream \a out.
   \since 1.1
 */
QDataStream& operator<<(QDataStream& out, const QOrganizerItem& item)
{
    quint8 formatVersion = 1; // Version of QDataStream format for QOrganizerItem
    out << formatVersion
        << item.id().toString()
        << item.collectionId().toString()
        << item.details();
    return out;
}

/*!
 * Reads an item from stream \a in into \a item.
   \since 1.1
 */
QDataStream& operator>>(QDataStream& in, QOrganizerItem& item)
{
    quint8 formatVersion;
    in >> formatVersion;
    if (formatVersion == 1) {
        item = QOrganizerItem();
        QString itemIdString;
        QString collectionIdString;
        QList<QOrganizerItemDetail> details;
        in >> itemIdString >> collectionIdString >> details;
        item.setId(QOrganizerItemId::fromString(itemIdString));
        item.setCollectionId(QOrganizerCollectionId::fromString(collectionIdString));
        item.d->m_details = details;
    } else {
        in.setStatus(QDataStream::ReadCorruptData);
    }
    return in;
}
#endif

/*!
    Returns the type of the organizer item.
 */
QOrganizerItemType::ItemType QOrganizerItem::type() const
{
    QOrganizerItemType newType = detail<QOrganizerItemType>();
    return newType.type();
}

/*!
    Sets the type of the organizer item to the given \a type.
 */
void QOrganizerItem::setType(QOrganizerItemType::ItemType type)
{
    QOrganizerItemType newType = detail<QOrganizerItemType>();
    newType.setType(type);
    newType.d->m_access = QOrganizerItemDetail::Irremovable;
    saveDetail(&newType);
}

/*!
 * Returns the display label of the item
   \since 1.1
 */
QString QOrganizerItem::displayLabel() const
{
    QOrganizerItemDisplayLabel dl = detail<QOrganizerItemDisplayLabel>();
    return dl.label();
}

/*!
 * Sets the display label of the item to \a label
 *
 * \sa QOrganizerItemDisplayLabel
   \since 1.1
 */
void QOrganizerItem::setDisplayLabel(const QString& label)
{
    QOrganizerItemDisplayLabel dl = detail<QOrganizerItemDisplayLabel>();
    dl.setLabel(label);
    saveDetail(&dl);
}

/*!
 * Returns the human-readable description of the item
 *
 * \sa QOrganizerItemDescription
   \since 1.1
 */
QString QOrganizerItem::description() const
{
    QOrganizerItemDescription descr = detail<QOrganizerItemDescription>();
    return descr.description();
}

/*!
 * Sets the human-readable description of the item to \a description
 *
 * \sa QOrganizerItemDescription
   \since 1.1
 */
void QOrganizerItem::setDescription(const QString& description)
{
    QOrganizerItemDescription descr = detail<QOrganizerItemDescription>();
    descr.setDescription(description);
    saveDetail(&descr);
}

/*!
 * Returns the list of comments (or arbitrary notes about the item)
 * which pertain to this item.
 *
 * \sa QOrganizerItemComment
   \since 1.1
 */
QStringList QOrganizerItem::comments() const
{
    QList<QOrganizerItemComment> comments = details<QOrganizerItemComment>();
    QStringList list;
    foreach (const QOrganizerItemComment& comment, comments) {
        list += comment.comment();
    }
    return list;
}

/*!
 * Removes all comments (arbitrary notes) about this item
 *
 * \sa QOrganizerItemComment
   \since 1.1
 */
void QOrganizerItem::clearComments()
{
    QList<QOrganizerItemComment> comments = details<QOrganizerItemComment>();
    foreach (QOrganizerItemComment comment, comments) {
        removeDetail(&comment);
    }
}

/*!
 * Sets the list of comments associated with the item to \a comments.
 *
 * \sa QOrganizerItemTag
   \since 1.2
 */
void QOrganizerItem::setComments(const QStringList& comments)
{
    d->removeOnly(QOrganizerItemComment::DefinitionName);
    foreach (const QString& comment, comments) {
        addComment(comment);
    }
}

/*!
 * Adds the comment \a comment to this item
 *
 * \sa QOrganizerItemComment
   \since 1.1
 */
void QOrganizerItem::addComment(const QString& comment)
{
    QOrganizerItemComment detail;
    detail.setComment(comment);
    saveDetail(&detail);
}

/*!
 * Returns the list of tags for this item.  Tags are used for non-exclusive categorization.
 *
 * \sa QOrganizerItemTag
   \since 1.2
 */
QStringList QOrganizerItem::tags() const
{
    QStringList tags;
    foreach (const QOrganizerItemTag& tagDetail, details<QOrganizerItemTag>()) {
        tags.append(tagDetail.tag());
    }
    return tags;
}

/*!
 * Removes all tags associated with the item.
 *
 * \sa QOrganizerItemTag
   \since 1.1
 */
void QOrganizerItem::clearTags()
{
    d->removeOnly(QOrganizerItemTag::DefinitionName);
}

/*!
 * Adds the \a tag to this item.
 *
 * \sa QOrganizerItemTag
   \since 1.2
 */
void QOrganizerItem::addTag(const QString& tag)
{
    QOrganizerItemTag tagDetail;
    tagDetail.setTag(tag);
    saveDetail(&tagDetail);
}

/*!
 * Sets the list of tags associated with the item to \a tags.
 *
 * \sa QOrganizerItemTag
   \since 1.2
 */
void QOrganizerItem::setTags(const QStringList& tags)
{
    d->removeOnly(QOrganizerItemTag::DefinitionName);
    foreach (const QString& tag, tags) {
        addTag(tag);
    }
}

/*!
 * Returns the globally unique identifier which identifies this item,
 * which is used for synchronization purposes.
   \since 1.1
 */
QString QOrganizerItem::guid() const
{
    QOrganizerItemGuid guid = detail<QOrganizerItemGuid>();
    return guid.guid();
}

/*!
 * Sets the item's globally unique identifier to \a guid
   \since 1.1
 */
void QOrganizerItem::setGuid(const QString& guid)
{
    QOrganizerItemGuid guidDetail = detail<QOrganizerItemGuid>();
    guidDetail.setGuid(guid);
    saveDetail(&guidDetail);
}

/*!
    Returns the data of the custom detail with the given \a name.
 */
QVariant QOrganizerItem::extendedDetailData(const QString &name) const
{
    QList<QOrganizerItemExtendedDetail> extendedDetails = details<QOrganizerItemExtendedDetail>();
    if (extendedDetails.size() > 0) {
        if (name.isEmpty())
            return extendedDetails.at(0).data();

        foreach (const QOrganizerItemExtendedDetail &detail, extendedDetails) {
            if (name == detail.name())
                return detail.data();
        }
    }
    return QVariant();
}

/*!
    Sets the \a data of a custom detail with the given \a name.
 */
void QOrganizerItem::setExtendedDetailData(const QString &name, const QVariant &data)
{
    QList<QOrganizerItemExtendedDetail> extendedDetails = details<QOrganizerItemExtendedDetail>();
    foreach (QOrganizerItemExtendedDetail detail, extendedDetails) {
        if (name == detail.name()) {
            detail.setData(data);
            return;
        }
    }
    QOrganizerItemExtendedDetail newDetail;
    newDetail.setName(name);
    newDetail.setData(data);
    saveDetail(&newDetail);
}

/* Helper functions for QOrganizerItemData */
void QOrganizerItemData::removeOnly(const QString& definitionName)
{
    QList<QOrganizerItemDetail>::iterator dit = m_details.begin();
    while (dit != m_details.end()) {
        // XXX this doesn't check type or display label
        if (dit->definitionName() == definitionName)
            dit = m_details.erase(dit);
        else
            ++dit;
    }
}

void QOrganizerItemData::removeOnly(const QSet<QString> &definitionNames)
{
    QList<QOrganizerItemDetail>::iterator dit = m_details.begin();
    while (dit != m_details.end()) {
        // XXX this doesn't check type or display label
        if (definitionNames.contains(dit->definitionName()))
            dit = m_details.erase(dit);
        else
            ++dit;
    }
}

QTORGANIZER_END_NAMESPACE
