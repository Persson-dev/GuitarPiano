#include "GPMusic.h"
#include "GPData.h"
#include "GPSave.h"

#include <map>

#define ARRAY_SIZE(A) sizeof(A) / sizeof(A[0])

namespace gpgui {
namespace music {

using ChordSave = save::ChordSave;


// The positions of strings in a piano keyboard
static const std::uint8_t Cordes[] = {
	7, 12, 17, 22, 26, 31
};

int GetStringOffset(int string) {
	return Cordes[string];
}

Note GetNote(std::uint8_t touche) {
	return Note(touche % 12);
}

std::uint8_t GetOctave(std::uint8_t touche) {
	return touche / 12 + 1;
}

std::string ToString(Note note) {
	switch (note) {
	case A:
		return "A";
	case Bb:
		return "A#";
	case B:
		return "B";
	case C:
		return "C";
	case Db:
		return "C#";
	case D:
		return "D";
	case Eb:
		return "D#";
	case E:
		return "E";
	case F:
		return "F";
	case Gb:
		return "F#";
	case G:
		return "G";
	case Ab:
		return "G#";
	default:
		return "wtf";
	}
}

std::string ToString(ChordType chord) {
	switch (chord) {
	case ChordType::Major:
		return "Majeur";
	case ChordType::Minor:
		return "Mineur";
	case ChordType::Dim:
		return "Dim";
	case ChordType::Sus:
		return "Sus";
	case ChordType::Major7:
		return "Majeur7";
	case ChordType::Minor7:
		return "Mineur7";
	default:
		return "";
	}
}

std::string ToString(const Tab& tab) {
	std::string result;
	for (int fret : tab) {
		if (fret == data::EMPTY_TAB) {
			result += "X";
		} else {
			result += std::to_string(fret);
		}
	}
	return result;
}

std::string ToString(const ChordSave& chord) {
	return ToString(chord.note) + " " + ToString(chord.type);
}

static bool isChord(int pos, const Chord& chord) {
	return GetNote(pos) == chord[0] || GetNote(pos) == chord[1] || GetNote(pos) == chord[2];
}

Tab FindChord(const Chord& chord, int capo) {
	Tab tab;
	tab.fill(data::EMPTY_TAB);

	for (int i = 0; i < tab.size(); i++) {
		int offset = 0;
		int cordePos = Cordes[i];

		while (!isChord(cordePos + offset + capo, chord)) {
			offset++;
		}
		tab[i] = offset;
		// if unable to have a note beacuse of the capo :
		if (offset > 12 - capo) {
			tab[i] = data::EMPTY_TAB;
		}
	}

	return tab;
}

Tab FindFakeChord(const Chord& chord, int capo, int pCorde) {
	Tab tab;
	tab.fill(data::EMPTY_TAB);

	for (int i = pCorde; i < tab.size(); i++) {
		int offset = 0;
		int cordePos = Cordes[i];

		while (GetNote(cordePos + offset + capo) != chord[(i - pCorde) % 3]) {
			offset++;
		}
		tab[i] = offset;
		if (offset > 12 - capo) {
			tab[i] = data::EMPTY_TAB;
		}
	}

	return tab;
}

Tab FindPianoChord(const ChordOffsets& notes, int capo, int fretThreshold) {
	Tab tab;
	tab.fill(data::EMPTY_TAB);
	int pCorde = 0;
	for (int noteIndex = 0; noteIndex < notes.size(); noteIndex++) {
		int note = notes[noteIndex];
		if (note == data::EMPTY_NOTE)  // empty chord
			continue;

		//searching a string
		for (int i = pCorde; i < sizeof(Cordes); i++) {
			if (note >= Cordes[i] + capo) {
				int fret = note - (Cordes[i] + capo);
				if (fret <= fretThreshold - capo) {
					pCorde = i;
					tab[i] = note - Cordes[i];

					if (note - (Cordes[i] + capo) == 0)  // the fret is on the capo
						tab[i] = 0;

					break;
				}
			}
		}
	}
	return tab;
}

Chord GetChord(Note note, ChordType type) {
	ChordOffsets offsets = GetChordOffsets(note, type);
	return { Note(note + offsets[0] % Note::TOTAL), Note(note + offsets[1] % Note::TOTAL), Note(note + offsets[2] % Note::TOTAL), offsets[3] == Note(data::EMPTY_NOTE) ? Note(data::EMPTY_NOTE) : Note(note + offsets[3]) };
}

ChordOffsets GetChordOffsets(Note note, ChordType type) {
	switch (type) {
	case ChordType::Major:
		return { 0, 4, 7, data::EMPTY_NOTE };
	case ChordType::Minor:
		return { 0, 3, 7, data::EMPTY_NOTE };
	case ChordType::Dim:
		return { 0, 3, 6, data::EMPTY_NOTE };
	case ChordType::Sus:
		return { 0, 5, 7, data::EMPTY_NOTE };
	case ChordType::Major7:
		return { 0, 4, 7, 10 };
	case ChordType::Minor7:
		return { 0, 3, 7, 10 };
	default:
		return {};
	}
}

} // namespace music
} // namespace gpgui