// SPDX-License-Identifier: GPL-2.0
#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include "core/dive.h"

#include <QUndoCommand>
#include <QCoreApplication> // For Q_DECLARE_TR_FUNCTIONS
#include <QVector>
#include <memory>

// The classes declared in this file represent units-of-work, which can be exectuted / undone
// repeatedly. The command objects are collected in a linear list implemented in the QUndoStack class.
// They contain the information that is necessary to either perform or undo the unit-of-work.
// The usage is:
//	constructor: generate information that is needed for executing the unit-of-work
//	redo(): performs the unit-of-work and generates the information that is needed for undo()
//	undo(): undos the unit-of-work and regenerates the initial information needed in redo()
// The needed information is mostly kept in pointers to dives and/or trips, which have to be added
// or removed.
// For this to work it is crucial that
// 1) Pointers to dives and trips remain valid as long as referencing command-objects exist.
// 2) The dive-table is not resorted, because dives are inserted at given indices.
//
// Thus, if a command deletes a dive or a trip, the actual object must not be deleted. Instead,
// the command object removes pointers to the dive/trip object from the backend and takes ownership.
// To reverse such a deletion, the object is re-injected into the backend and ownership is given up.
// Once ownership of a dive is taken, any reference to it was removed from the backend. Thus,
// subsequent redo()/undo() actions cannot access this object and integrity of the data is ensured.
//
// As an example, consider the following course of events: Dive 1 is renumbered and deleted, dive 2
// is added and renumbered. The undo list looks like this (---> non-owning, ***> owning pointers,
// ===> next item in list)
//
//                                        Undo-List
// +-----------------+     +---------------+     +------------+     +-----------------+
// | Renumber dive 1 |====>| Delete dive 1 |====>| Add dive 2 |====>| Renumber dive 2 |
// +------------------     +---------------+     +------------+     +-----------------+
//          |                      *                   |                     |
//          |      +--------+      *                   |    +--------+       |
//          +----->| Dive 1 |<******                   +--->| Dive 2 |<------+
//                 +--------+                               +--------+
//                                                              ^
//                                    +---------+               *
//                                    | Backend |****************
//                                    +---------+
// Two points of note:
// 1) Every dive is owned by either the backend or exactly one command object.
// 2) All references to dive 1 are *before* the owner "delete dive 2", thus the pointer is always valid.
// 3) References by the backend are *always* owning.
//
// The user undos the last two commands. The situation now looks like this:
//
//
//                  Undo-List                                Redo-List
// +-----------------+     +---------------+     +------------+     +-----------------+
// | Renumber dive 1 |====>| Delete dive 1 |     | Add dive 2 |<====| Renumber dive 2 |
// +------------------     +---------------+     +------------+     +-----------------+
//          |                      *                   *                     |
//          |      +--------+      *                   *    +--------+       |
//          +----->| Dive 1 |<******                   ****>| Dive 2 |<------+
//                 +--------+                               +--------+
//
//                                    +---------+
//                                    | Backend |
//                                    +---------+
// Again:
// 1) Every dive is owned by either the backend (here none) or exactly one command object.
// 2) All references to dive 1 are *before* the owner "delete dive 1", thus the pointer is always valid.
// 3) All references to dive 2 are *after* the owner "add dive 2", thus the pointer is always valid.
//
// The user undos one more command:
//
//       Undo-List                                 Redo-List
// +-----------------+     +---------------+     +------------+     +-----------------+
// | Renumber dive 1 |     | Delete dive 1 |<====| Add dive 2 |<====| Renumber dive 2 |
// +------------------     +---------------+     +------------+     +-----------------+
//          |                      |                   *                     |
//          |      +--------+      |                   *    +--------+       |
//          +----->| Dive 1 |<-----+                   ****>| Dive 2 |<------+
//                 +--------+                               +--------+
//                     ^
//                     *              +---------+
//                     ***************| Backend |
//                                    +---------+
// Same points as above.
// The user now adds a dive 3. The redo list will be deleted:
//
//                                      Undo-List
// +-----------------+                                              +------------+
// | Renumber dive 1 |=============================================>| Add dive 3 |
// +------------------                                              +------------+
//          |                                                             |
//          |      +--------+                               +--------+    |
//          +----->| Dive 1 |                               | Dive 3 |<---+
//                 +--------+                               +--------+
//                     ^                                        ^
//                     *              +---------+               *
//                     ***************| Backend |****************
//                                    +---------+
// Note:
// 1) Dive 2 was deleted with the "add dive 2" command, because that was the owner.
// 2) Dive 1 was not deleted, because it is owned by the backend.
//
// To take ownership of dives/trips, the OnwingDivePtr and OwningTripPtr types are used. These
// are simply derived from std::unique_ptr and therefore use well-established semantics.
// Expressed in C-terms: std::unique_ptr<T> is exactly the same as T* with the following
// twists:
// 1) default-initialized to NULL.
// 2) if it goes out of scope (local scope or containing object destroyed), it does:
//	if (ptr) free_function(ptr);
//    whereby free_function can be configured (defaults to delete ptr).
// 3) assignment between two std::unique_ptr<T> compiles only if the source is reset (to NULL).
//    (hence the name - there's a *unique* owner).
// While this sounds trivial, experience shows that this distinctly simplifies memory-management
// (it's not necessary to manually delete all vector items in the destructur, etc).
// Note that Qt's own implementation (QScoperPointer) is not up to the job, because it doesn't implement
// move-semantics and Qt's containers are incompatible, owing to COW semantics.
//
// Usage:
//	OwningDivePtr dPtr;			// Initialize to null-state: not owning any dive.
//	OwningDivePtr dPtr(dive);		// Take ownership of dive (which is of type struct dive *).
//						// If dPtr goes out of scope, the dive will be freed with free_dive().
//	struct dive *d = dPtr.release();	// Give up ownership of dive. dPtr is reset to null.
//	struct dive *d = d.get();		// Get pointer dive, but don't release ownership.
//	dPtr.reset(dive2);			// Delete currently owned dive with free_dive() and get ownership of dive2.
//	dPtr.reset();				// Delete currently owned dive and reset to null.
//	dPtr2 = dPtr1;				// Fails to compile.
//	dPtr2 = std::move(dPtr1);		// dPtr2 takes ownership, dPtr1 is reset to null.
//	OwningDivePtr fun();
//	dPtr1 = fun();				// Compiles. Simply put: the compiler knows that the result of fun() will
//						// be trashed and therefore can be moved-from.
//	std::vector<OwningDivePtr> v:		// Define an empty vector of owning pointers.
//	v.emplace_back(dive);			// Take ownership of dive and add at end of vector
//						// If the vector goes out of scope, all dives will be freed with free_dive().
//	v.clear(v);				// Reset the vector to zero length. If the elements weren't release()d,
//						// the pointed-to dives are freed with free_dive()

// Classes used to automatically call free_dive()/free_trip for owning pointers that go out of scope.
struct DiveDeleter {
	void operator()(dive *d) { free_dive(d); }
};
struct TripDeleter {
	void operator()(dive_trip *t) { free_trip(t); }
};

// Owning pointers to dive and dive_trip objects.
typedef std::unique_ptr<dive, DiveDeleter> OwningDivePtr;
typedef std::unique_ptr<dive_trip, TripDeleter> OwningTripPtr;

// This helper structure describes a dive that we want to add.
// Potentially it also adds a trip (if deletion of the dive resulted in deletion of the trip)
struct DiveToAdd {
	OwningDivePtr	 dive;		// Dive to add
	OwningTripPtr	 tripToAdd;	// Not-null if we also have to add a dive
	dive_trip	*trip;		// Trip the dive belongs to, may be null
	int		 idx;		// Position in divelist
};

// This helper structure describes a dive that should be moved to / removed from
// a trip. If the "trip" member is null, the dive is removed from its trip (if
// it is in a trip, that is)
struct DiveToTrip
{
	struct dive	*dive;
	dive_trip	*trip;
};

// This helper structure describes a number of dives to add to /remove from /
// move between trips.
// It has ownership of the trips (if any) that have to be added before hand.
struct DivesToTrip
{
	std::vector<DiveToTrip> divesToMove;		// If dive_trip is null, remove from trip
	std::vector<OwningTripPtr> tripsToAdd;
};

class UndoAddDive : public QUndoCommand {
public:
	UndoAddDive(dive *dive);
private:
	void undo() override;
	void redo() override;

	// For redo
	DiveToAdd	diveToAdd;

	// For undo
	dive		*diveToRemove;
};

class UndoDeleteDive : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(Command)
public:
	UndoDeleteDive(const QVector<dive *> &divesToDelete);
private:
	void undo() override;
	void redo() override;

	// For redo
	std::vector<struct dive*> divesToDelete;

	std::vector<OwningTripPtr> tripsToAdd;
	std::vector<DiveToAdd> divesToAdd;
};

class UndoShiftTime : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(Command)
public:
	UndoShiftTime(const QVector<dive *> &changedDives, int amount);
private:
	void undo() override;
	void redo() override;

	// For redo and undo
	QVector<dive *> diveList;
	int timeChanged;
};

class UndoRenumberDives : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(Command)
public:
	UndoRenumberDives(const QVector<QPair<int, int>> &divesToRenumber);
private:
	void undo() override;
	void redo() override;

	// For redo and undo: pairs of dive-id / new number
	QVector<QPair<int, int>> divesToRenumber;
};

// The classes UndoRemoveDivesFromTrip, UndoRemoveAutogenTrips, UndoCreateTrip,
// UndoAutogroupDives and UndoMergeTrips all do the same thing, just the intialization
// differs. Therefore, define a base class with the proper data-structures, redo()
// and undo() functions and derive to specialize the initialization.
class UndoTripBase : public QUndoCommand {
	Q_DECLARE_TR_FUNCTIONS(Command)
protected:
	void undo() override;
	void redo() override;

	// For redo and undo
	DivesToTrip divesToMove;
};
struct UndoRemoveDivesFromTrip : public UndoTripBase {
	UndoRemoveDivesFromTrip(const QVector<dive *> &divesToRemove);
};
struct UndoRemoveAutogenTrips : public UndoTripBase {
	UndoRemoveAutogenTrips();
};
struct UndoAddDivesToTrip : public UndoTripBase {
	UndoAddDivesToTrip(const QVector<dive *> &divesToAdd, dive_trip *trip);
};
struct UndoCreateTrip : public UndoTripBase {
	UndoCreateTrip(const QVector<dive *> &divesToAdd);
};
struct UndoAutogroupDives : public UndoTripBase {
	UndoAutogroupDives();
};
struct UndoMergeTrips : public UndoTripBase {
	UndoMergeTrips(dive_trip *trip1, dive_trip *trip2);
};

class UndoSplitDives : public QUndoCommand {
public:
	// If time is < 0, split at first surface interval
	UndoSplitDives(dive *d, duration_t time);
private:
	void undo() override;
	void redo() override;

	// For redo
	// For each dive to split, we remove one from and put two dives into the backend
	dive		*diveToSplit;
	DiveToAdd	 splitDives[2];

	// For undo
	// For each dive to unsplit, we remove two dives from and add one into the backend
	DiveToAdd	 unsplitDive;
	dive		*divesToUnsplit[2];
};

class UndoMergeDives : public QUndoCommand {
public:
	UndoMergeDives(const QVector<dive *> &dives);
private:
	void undo() override;
	void redo() override;

	// For redo
	// Add one and remove a batch of dives
	DiveToAdd		 mergedDive;
	std::vector<dive *>	 divesToMerge;

	// For undo
	// Remove one and add a batch of dives
	dive			*diveToUnmerge;
	std::vector<DiveToAdd>	 unmergedDives;

	// For undo and redo
	QVector<QPair<int, int>> divesToRenumber;
};

#endif // UNDOCOMMANDS_H
