#include "Regex.h"
//map<string, Regex::Cache> Regex::cache;

Regex::Regex()
{
	memset(&regex, 0, sizeof(regex));
}

Regex::Regex(char const *rx)
{
	*this = rx;
}

Regex::Regex(Regex const &copy)
{
	*this = copy;
}

Regex::~Regex()
{
	// Free RX if not in cache
	//if (cache.find(inrx) == cache.end())
	if (inrx != "") regfree(&regex);
	/*else
		cache[inrx].instances--;*/
}

Regex &Regex::operator = (Regex const &copy)
{
	return (*this = copy.inrx.c_str());
}

Regex &Regex::operator = (char const *rx)
{
int error;

	if (!rx || !rx[0]) return *this;

	inrx = rx;
	
/*map<string, Cache>::iterator hit;

	if ((hit = cache.find(rx)) != cache.end())
	{
		regex = (*hit).second.rx;
		(*hit).second.hits++;
		(*hit).second.instances++;
		return *this;
	}*/

	if ((error = regcomp(&regex, inrx.c_str(), REG_EXTENDED | REG_NEWLINE)))
	{
	char buffer[128];
		regerror(error, &regex, buffer, 128);
		throw runtime_error("couldn't compile rx: " + string(buffer));
	}

/*Cache &c = cache[rx];

	c.rx = regex;
	c.hits++;
	c.instances = 1;

	// Erase least used entry
	if (cache.size() >= CRASH_REGEX_CACHE_THRESHOLD)
	{
		for (map<string, Cache>::iterator i = cache.begin(); i != cache.end(); i++)
			if ((*i).second.hits < (*hit).second.hits)
				hit = i;
		// Free RX if not in use
		if ((*hit).second.instances == 0)
			regfree(&(*hit).second.rx);
		cache.erase(hit);
	}*/

	return *this;
}

int Regex::match(char const *str)
{
	if (regexec(&regex, str, 50, matches, 0) == REG_NOMATCH) return -1;
	return matches[0].rm_eo - matches[0].rm_so;
}

int Regex::matchStart(char const *str)
{
	if (match(str) == -1) return -1;
	if (matches[0].rm_so != 0) return -1;
	return matches[0].rm_eo;
}

string Regex::transform(string const &str, string const &mask) {
	if (match(str.c_str()) == -1) throw no_match("couldn't transform '" + str + "' with '" + inrx + "' as it does not match");
int count = 10;
string token;

	for (int i = 0; i < 10; i++)
		if (matches[i].rm_so == -1) {
			count = i;
			break;
		}

	for (unsigned i = 0; i < mask.size(); i++)
		if (mask[i] == '\\' && strchr("0123456789", mask[i + 1]))
		{
		int index = mask[i + 1] - '0';

			if (index >= count)
				throw out_of_range("Regex transform index for '" + inrx + "' out of range (" + mask.substr(i + 1, 1) + ")");
			else
				token += string(str, matches[index].rm_so, matches[index].rm_eo - matches[index].rm_so);
			i++;
		} else
			token += mask[i];
	return token;
}
