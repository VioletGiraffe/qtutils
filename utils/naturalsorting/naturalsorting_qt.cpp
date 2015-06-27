#include "naturalsorting_qt.h"

bool naturalSortComparisonQt(const QString& s1, const QString& s2, SortingOptions options)
{
	// ignore common prefix..
	int i = 0;
	while (i < s1.length() && i < s2.length() && s1[i].toLower() == s2[i].toLower())
		++i;
	++i;

	// something left to compare?
	if (i < s1.length() && i < s2.length())
	{
		// get number prefix from position i - doesn't matter from which string
		int k = i-1;
		//If not number return native comparator
		if(!s1[k].isNumber() || !s2[k].isNumber())
		{
			//Two next lines
			//E.g. 1_... < 12_...
			if(s1[k].isNumber())
				return !options._digitsAfterLetters;
			if(s2[k].isNumber())
				return options._digitsAfterLetters;

			return QString::compare(s1, s2, Qt::CaseInsensitive) < 0;
		}

		QString n;
		--k;
		while (k >= 0 && s1[k].isNumber())
		{
			n = s1[k]+n;
			--k;
		}

		// get relevant/significant number string for s1
		k = i-1;
		QString n1;
		while (k < s1.length() && s1[k].isNumber())
		{
			n1 += s1.at(k);
			++k;
		}

		// get relevant/significant number string for s2
		//Decrease by
		k = i-1;
		QString n2;
		while (k < s2.length() && s2[k].isNumber())
		{
			n2 += s2.at(k);
			++k;
		}

		// got two numbers to compare?
		if (!n1.isEmpty() && !n2.isEmpty())
			return (n+n1).toInt() < (n+n2).toInt();
		else
		{
			// not a number has to win over a number.. number could have ended earlier... same prefix..
			if (!n1.isEmpty())
				return options._digitsAfterLetters;
			if (!n2.isEmpty())
				return !options._digitsAfterLetters;
			return s1[i] < s2[i];
		}
	}
	else
	{
		// shortest string win
		return s1.length() < s2.length();
	}
}
