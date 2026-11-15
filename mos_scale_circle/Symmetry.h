/*
  ==============================================================================

    Symmetry.h
    Created: 17 Jun 2020 9:54:26pm
    Author:  Vincenzo Sicurella

  ==============================================================================
*/

#pragma once

/*
	Takes an array and arranges it to wrap into a circle that is symmetric about arrayOfValues[centerIndex],
	which becomes element 0 of the result. If no symmetric arrangement exists (more than one value appears
	an odd number of times among the non-center values), an empty array is returned.

	The non-center values form a palindrome: one half, the odd-count leftover (if any) at the bottom of the
	circle, then the half mirrored. The half is built one slot at a time, choosing the value that, in order
	of priority:
		1. keeps the fewest adjacent equal values around the whole circle (always the minimum possible),
		2. avoids placing the center's own value right beside the center,
		3. spreads values evenly around the circle: the most common value is spaced against all others,
		   then each value is placed closest to its evenly-spaced ideal position,
		4. is the smaller value (so smaller values tend to sit nearer the center).

	ex. [7, 7, 7, 5, 5]    => [7, 5, 7, 7, 5]
	    [7, 7, 7, 5, 5, 5] => [7, 5, 7, 5, 7, 5]
*/
static Array<int> arrangeSymmetrically(Array<int> arrayOfValues, int centerIndex=0)
{
	const int centerValue = arrayOfValues[centerIndex];
	arrayOfValues.remove(centerIndex);

	// unique values (ascending) and how many times each appears
	Array<int> values;
	for (auto v : arrayOfValues)
		values.addIfNotAlreadyThere(v);
	values.sort();

	const int numValues = values.size();
	Array<int> counts;
	counts.insertMultiple(0, 0, numValues);
	for (auto v : arrayOfValues)
		counts.getReference(values.indexOf(v))++;

	// the non-center values must form a palindrome, so at most one value may have an odd count
	int middleIndex = -1;
	for (int i = 0; i < numValues; i++)
	{
		if (counts[i] % 2 == 1)
		{
			if (middleIndex >= 0)
				return Array<int>();
			middleIndex = i;
		}
	}

	const bool hasMiddle = middleIndex >= 0;
	const int middleValue = hasMiddle ? values[middleIndex] : 0;

	// per-value counts for the half, and totals around the whole circle (including the center)
	Array<int> halfCounts, circleCounts, placed;
	int halfSize = 0;
	for (int i = 0; i < numValues; i++)
	{
		const int isCenter = values[i] == centerValue ? 1 : 0;
		halfCounts.add(counts[i] / 2);
		circleCounts.add(counts[i] + isCenter);
		placed.add(isCenter);
		halfSize += counts[i] / 2;
	}

	const int circleSize = arrayOfValues.size() + 1;

	// The most common value (smaller value on ties); everything else separates its runs.
	int dominantValue = centerValue;
	int dominantCount = values.contains(centerValue) ? 0 : 1;
	for (int i = 0; i < numValues; i++)
	{
		if (circleCounts[i] > dominantCount || (circleCounts[i] == dominantCount && values[i] < dominantValue))
		{
			dominantValue = values[i];
			dominantCount = circleCounts[i];
		}
	}

	// Separators (non-dominant values) are spaced evenly from the center, Bresenham style.
	// The phase is in half-slots: the center is a separator position unless it is the dominant value.
	const int numSeparators = circleSize - dominantCount;
	const int separatorPhase = centerValue != dominantValue ? 0 : 1;
	auto separatorsBefore = [&](int slot) { return ((2 * slot + separatorPhase) * numSeparators) / (2 * circleSize); };
	auto wantsSeparator = [&](int slot) { return numSeparators > 0 && separatorsBefore(slot) > separatorsBefore(slot - 1); };

	// Lower bound on adjacent equal pairs in the sequence: first, (any order of) remaining, last (if hasLast).
	// A value's runs must be split by other values, so it can't have more runs than its separators allow.
	auto minAdjacentDuplicates = [&](int first, const Array<int>& remaining, bool hasLast, int last)
	{
		int remainingSize = 0;
		for (auto c : remaining)
			remainingSize += c;

		const int length = 1 + remainingSize + (hasLast ? 1 : 0);
		int bound = 0;
		for (int i = 0; i < numValues; i++)
		{
			const int v = values[i];
			const int count = remaining[i] + (first == v ? 1 : 0) + (hasLast && last == v ? 1 : 0);
			if (count == 0)
				continue;

			const bool lastCanBeV = hasLast ? last == v : (remaining[i] > 0 || (remainingSize == 0 && first == v));
			const int endsNotV = (first != v ? 1 : 0) + (lastCanBeV ? 0 : 1);
			const int maxRuns = (length - count) + 1 - endsNotV;
			bound = jmax(bound, count - maxRuns);
		}
		return bound;
	};

	// Build the half, from beside the center down toward the bottom of the circle
	Array<int> half;
	int previous = centerValue;
	for (int slot = 1; slot <= halfSize; slot++)
	{
		const bool separatorSlot = wantsSeparator(slot);

		int bestIndex = -1;
		int bestDuplicates = 0, bestFlank = 0, bestMismatch = 0;
		int64 bestNextNum = 0, bestNextDen = 1;

		for (int i = 0; i < numValues; i++)
		{
			if (halfCounts[i] == 0)
				continue;

			const int v = values[i];

			halfCounts.getReference(i)--;
			const int duplicates = (v == previous ? 1 : 0) + minAdjacentDuplicates(v, halfCounts, hasMiddle, middleValue);
			halfCounts.getReference(i)++;

			const int flank = (slot == 1 && v == centerValue) ? 1 : 0;
			const int mismatch = ((v != dominantValue) != separatorSlot) ? 1 : 0;

			// next evenly-spaced ideal position of v: (placed + phase) * circleSize / circleCount,
			// with phase 0 for the center's value (it sits at position 0) and 1/2 otherwise
			const int64 nextNum = (int64) (2 * placed[i] + (v == centerValue ? 0 : 1)) * circleSize;
			const int64 nextDen = (int64) 2 * circleCounts[i];

			bool better = bestIndex < 0;
			if (! better)
			{
				if (duplicates != bestDuplicates)
					better = duplicates < bestDuplicates;
				else if (flank != bestFlank)
					better = flank < bestFlank;
				else if (mismatch != bestMismatch)
					better = mismatch < bestMismatch;
				else
					better = nextNum * bestNextDen < bestNextNum * nextDen; // ties keep the smaller value
			}

			if (better)
			{
				bestIndex = i;
				bestDuplicates = duplicates;
				bestFlank = flank;
				bestMismatch = mismatch;
				bestNextNum = nextNum;
				bestNextDen = nextDen;
			}
		}

		half.add(values[bestIndex]);
		halfCounts.getReference(bestIndex)--;
		placed.getReference(bestIndex)++;
		previous = values[bestIndex];
	}

	Array<int> symmetricArray = { centerValue };
	symmetricArray.addArray(half);

	if (hasMiddle)
		symmetricArray.add(middleValue);

	for (int i = half.size() - 1; i >= 0; i--)
		symmetricArray.add(half[i]);

	return symmetricArray;
}

template <class T>
static bool isSymmetric(Array<T> arrayIn)
{
	int indNext, indPrev, symCount;
	int halfSize = arrayIn.size();

	for (int i = 0; i < arrayIn.size(); i++)
	{
		symCount = 0;
		for (int j = 1; j <= halfSize; j++)
		{
			indNext = (i + j) % arrayIn.size();
			indPrev = (i - j);
			indPrev += indPrev < 0 ? arrayIn.size() : 0;

			if (arrayIn[indNext] == arrayIn[indPrev])
			{
				symCount++;
			}
		}

		if (symCount == halfSize)
			return true;
	}
	return false;
}
