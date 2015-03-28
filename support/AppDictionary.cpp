#include "support\AppDictionary.h"

USING_NS_CC;

// singleton objects
AppDictionary*	AppDictionary::s_dictionary = nullptr;
std::string		AppDictionary::s_extention	= ".dict";


AppDictionary::AppDictionary()
{
}

void	AppDictionary::init()
{
	s_dictionary->loadAll("dictionaries/");
}

AppDictionary*	AppDictionary::getInstance()
{
	if (!s_dictionary)
	{
		s_dictionary = new AppDictionary();
		s_dictionary->init();
	}

	return s_dictionary;
}

void	AppDictionary::loadAll(const std::string& path)
{
	this->loadKanaRows(path + "kana_rows" + s_extention);
	this->loadKanji(path + "kanji" + s_extention);
	this->loadWords(path + "words" + s_extention);
	this->loadItems(path + "items" + s_extention);
	this->loadKanaKeyboardTable(path + "kana_keyboard_table" + s_extention);
}

void	AppDictionary::loadKanaRows(const std::string& filename)
{
	std::ifstream	inputFile(filename);
	std::string		inputString;

	while (std::getline(inputFile, inputString))
	{
		std::vector<std::string> kanaRow;
		int spacePos = inputString.find(' ');
		while (spacePos != std::string::npos)
		{
			kanaRow.push_back(inputString.substr(0, spacePos));
			inputString = inputString.substr(spacePos + 1, inputString.length() - (spacePos + 1));
			spacePos = inputString.find(' ');
		}
		kanaRow.push_back(inputString);

		m_kanaRowsDictionary.push_back(kanaRow);
	}
}

void	AppDictionary::loadKanji(const std::string& filename)
{
	std::ifstream	inputFile(filename);
	std::string		inputString;

	while (std::getline(inputFile, inputString))
	{
		Kanji* kanji = new Kanji(inputString);
		m_kanjiDictionary[ kanji->getID() ] = kanji;
	}
}

void	AppDictionary::loadWords(const std::string& filename)
{
	std::ifstream	inputFile(filename);
	std::string		inputString;

	while (std::getline(inputFile, inputString))
	{
		DictionaryWord* word = new DictionaryWord(inputString);
		m_wordsDictionary[ word->getID() ] = word;
	}
}

void	AppDictionary::loadItems(const std::string& filename)
{
	/*std::ifstream	inputFile(filename);
	std::string		inputString;

	while (std::getline(inputFile, inputString))
	{
		InventoryItem* item = new InventoryItem(inputString);
		m_itemsDictionary[ item->getID() ] = item;
	}*/
}

void	AppDictionary::loadKanaKeyboardTable(const std::string& filename)
{
	std::ifstream	inputFile(filename);
	std::string		inputString;

	while (std::getline(inputFile, inputString))
	{
		std::string key, hiragana, katakana, romaji;
		short spacePos = inputString.find(' ');

		key = inputString.substr(0, spacePos);
		inputString = inputString.substr(spacePos + 1, inputString.length() - (spacePos + 1));

		spacePos = inputString.find(' ');
		hiragana = inputString.substr(0, spacePos);
		inputString = inputString.substr(spacePos + 1, inputString.length() - (spacePos + 1));
		
		spacePos = inputString.find(' ');
		katakana = inputString.substr(0, spacePos);
		romaji = inputString.substr(spacePos + 1, inputString.length() - (spacePos + 1));


		Triad<std::string, std::string, std::string> kanaTriad(hiragana, katakana, romaji);
		this->m_keyboardDictionary[key] = kanaTriad;
	}
}

std::vector<std::vector<std::string>>	AppDictionary::getKanaRowsDictionary()
{
	return m_kanaRowsDictionary;
}

std::map<int, Kanji*>	AppDictionary::getKanjiDictionary()
{
	return m_kanjiDictionary;
}

std::map<int, DictionaryWord*>	AppDictionary::getWordsDictionary()
{
	return m_wordsDictionary;
}

std::map<int, InventoryItem*>	AppDictionary::getItemsDictionary()
{
	return m_itemsDictionary;
}

std::map<std::string, Triad<std::string, std::string, std::string>>	AppDictionary::getKeyboardDictionary()
{
	return m_keyboardDictionary;
}

std::vector<DictionaryWord*>	AppDictionary::getWordsByRomaji(const std::string& romaji)
{
	std::vector<DictionaryWord*> result;

	for (std::map<int, DictionaryWord*>::iterator it = m_wordsDictionary.begin(); it != m_wordsDictionary.end(); it++)
	{
		if (it->second->getRomajiWriting() == romaji)
		{
			result.push_back(it->second);
		}
	}

	return result;
}

std::vector<DictionaryWord*>	AppDictionary::getWordsByKana(const std::string& kana)
{
	std::vector<DictionaryWord*> result;

	for (std::map<int, DictionaryWord*>::iterator it = m_wordsDictionary.begin(); it != m_wordsDictionary.end(); it++)
	{
		if (it->second->getKanaWriting() == kana)
		{
			result.push_back(it->second);
		}
	}

	return result;
}

std::string	AppDictionary::getHiraganaByLatin(const std::string& key)
{
	return m_keyboardDictionary[key].first;
}

std::string	AppDictionary::getKatakanaByLatin(const std::string& key)
{
	return m_keyboardDictionary[key].second;
}

std::string	AppDictionary::getKeyButtonByRomaji(const std::string& romaji)
{
	for (std::map<std::string, Triad<std::string, std::string, std::string>>::iterator it = m_keyboardDictionary.begin();
		it != m_keyboardDictionary.end(); it++)
	{
		if (it->second.third == romaji)
		{
			return it->first;
		}
	}

	return nullptr;
}