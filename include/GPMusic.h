#pragma once

#include <string>
#include <array>
#include <vector>

namespace gpgui {

namespace save {

struct ChordSave;

} // namespace save

namespace music {

enum Note : std::uint8_t {
	A = 0,
	Bb,
	B,
	C,
	Db,
	D,
	Eb,
	E,
	F,
	Gb,
	G,
	Ab,

	TOTAL
};

enum class ChordType : std::uint8_t {
	Major = 0,
	Minor,
	Dim,
	Major7,
	Minor7,
	Sus,

	COUNT
};

typedef std::array<int, 6> Tab;
typedef std::array<Note, 4> Chord;
typedef std::array<std::uint8_t, 4> ChordOffsets;

Note GetNote(std::uint8_t touche);
std::uint8_t GetOctave(std::uint8_t touche);

std::string ToString(Note note);
std::string ToString(ChordType chord);
std::string ToString(const Tab& tab);
std::string ToString(const save::ChordSave& tab);

int GetStringOffset(int string);

Tab FindChord(const Chord& chord, int capo);
Tab FindFakeChord(const Chord& chord, int capo, int pCorde = 0);

Tab FindPianoChord(const ChordOffsets& notes, int capo = 0, int fretThreshold = 5);

ChordOffsets GetChordOffsets(Note note, ChordType type);
Chord GetChord(Note note, ChordType type);

} // namespace music
} // namespace gpgui