#include "panes\interface\DictionaryInterface.h"

USING_NS_CC;


DictionaryInterface::DictionaryInterface()
{
	m_tabLayer = Layer::create();

	Sprite* dictionaryPageOne = Sprite::create("textures/interface/dictionary/dictionary_placeholder.jpg");
	Sprite* dictionaryPageTwo = Sprite::create("textures/interface/dictionary/dictionary_placeholder_2.jpg");
	dictionaryPageOne->setAnchorPoint(Vec2(0, 0));
	dictionaryPageTwo->setAnchorPoint(Vec2(0, 0));
	dictionaryPageOne->setPosition(Vec2(c_alignOffset, c_alignOffset));
	dictionaryPageTwo->setPosition(Vec2(dictionaryPageOne->getPosition().x + dictionaryPageOne->getContentSize().width,
		dictionaryPageOne->getPosition().y));

	auto kanji = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::tabmenuCallback, this));
	auto words = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::tabmenuCallback, this));
	auto grammar = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::tabmenuCallback, this));
	auto kana = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::tabmenuCallback, this));

	kanji->setPosition(Vec2(kanji->getContentSize().width / 2,
		dictionaryPageOne->getPosition().y + dictionaryPageOne->getContentSize().height));
	words->setPosition(Vec2(kanji->getPosition().x + kanji->getContentSize().width, kanji->getPosition().y));
	grammar->setPosition(Vec2(words->getPosition().x + words->getContentSize().width, words->getPosition().y));
	kana->setPosition(Vec2(grammar->getPosition().x + grammar->getContentSize().width, grammar->getPosition().y));

	kanji->setTag(DB_KANJI);
	words->setTag(DB_WORDS);
	grammar->setTag(DB_GRAMMAR);
	kana->setTag(DB_KANA);

	auto lKanji = Label::create("Kanji", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lWords = Label::create("Words", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lGrammar = Label::create("Grammar", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lKana = Label::create("Kana", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	lKanji->setPosition(kanji->getPosition().x + c_alignOffset, kanji->getPosition().y + c_alignOffset);
	lWords->setPosition(words->getPosition().x + c_alignOffset, words->getPosition().y + c_alignOffset);
	lGrammar->setPosition(grammar->getPosition().x + c_alignOffset, grammar->getPosition().y + c_alignOffset);
	lKana->setPosition(kana->getPosition().x + c_alignOffset, kana->getPosition().y + c_alignOffset);

	this->addChild(lKanji, DICTIONARY_TAB_LAYER_ORDER, DL_KANJI);
	this->addChild(lWords, DICTIONARY_TAB_LAYER_ORDER, DL_WORDS);
	this->addChild(lGrammar, DICTIONARY_TAB_LAYER_ORDER, DL_GRAMMAR);
	this->addChild(lKana, DICTIONARY_TAB_LAYER_ORDER, DL_KANA);

	Menu* tabMenu = Menu::create(kanji, words, grammar, kana, nullptr);
	tabMenu->setPosition(Vec2(dictionaryPageOne->getPosition().x,
		dictionaryPageOne->getPosition().y));
	this->addChild(tabMenu, DICTIONARY_MENU_ORDER, DICTIONARY_TABMENU);
	
	this->addChild(dictionaryPageOne, DICTIONARY_PAGE_ORDER, DICTIONARY_PAGE_ONE);
	this->addChild(dictionaryPageTwo, DICTIONARY_PAGE_ORDER, DICTIONARY_PAGE_TWO);
	
	// <OTHER_INIT>
	m_currentTab = NULL;
	kanji->activate();
	this->switchCurrentTab(DB_KANJI);

	m_currentKanjiInputType = DICTIONARY_KANJI_INPUT_TYPE_IMAGE;

	m_kanaPageBrushDepth = MAX_BRUSH_DEPTH_PX - 2;
	// </OTHER INIT>
}

DictionaryInterface::~DictionaryInterface()
{
	this->removeAllChildrenWithCleanup(true);
	this->removeFromParentAndCleanup(true);
}

DictionaryInterface*	DictionaryInterface::create()
{
	DictionaryInterface* dictionaryInterface = new DictionaryInterface();

	if (dictionaryInterface->init())
	{
		dictionaryInterface->autorelease();
		return dictionaryInterface;
	}

	CC_SAFE_DELETE(dictionaryInterface);
	return nullptr;
}

void	DictionaryInterface::switchCurrentTab(BYTE tabTag)
{
	if (tabTag == m_currentTab)
		return;

	if (m_tabLayer->getChildrenCount())
	{
		m_tabLayer->removeFromParentAndCleanup(true);
		m_tabLayer = Layer::create();

		// CLEAR OLD TABS DATA
		this->unschedule(schedule_selector(DictionaryInterface::onEachFrameWordTab));
		m_kanaString.clear();
		//

		// SET DEFAULT KANA TYPE
		m_currentKanaType = DICTIONARY_KANA_TYPE_HIRAGANA;
		m_currentKanjiInputType = DICTIONARY_KANJI_INPUT_TYPE_IMAGE;
		//
	}

	switch (tabTag)
	{
	case DB_KANJI:
		_createKanjiTabImageSearch();
		break;
	case DB_WORDS:
		_createWordsTab();
		break;
	case DB_GRAMMAR:
		_createGrammarTab();
		break;
	case DB_KANA:
		_createKanaTab();
		break;
	}

	m_currentTab = tabTag;
	this->addChild(m_tabLayer, DICTIONARY_TAB_LAYER_ORDER, DICTIONARY_TAB_LAYER);
}

// <CALLBACKS>
void	DictionaryInterface::tabmenuCallback(Ref* pSender)
{
	BYTE tabTag = ((MenuItem*) pSender)->getTag();

	short prevTab = m_currentTab;
	MenuItemImage* senderButton = (MenuItemImage*)pSender;
	MenuItemImage* prevSender = (MenuItemImage*)senderButton->getParent()->getChildByTag(prevTab);
	
	if (prevSender != nullptr)
		prevSender->unselected();

	senderButton->selected();
	prevSender = senderButton;

	switchCurrentTab(senderButton->getTag());
}

void	DictionaryInterface::kanaTypeMenuCallback(Ref* pSender)
{
	short prevKanaType = m_currentKanaType;
	MenuItemImage* senderButton = (MenuItemImage*)pSender;
	MenuItemImage* prevSender = (MenuItemImage*)senderButton->getParent()->getChildByTag(prevKanaType);

	m_currentKanaType = senderButton->getTag();
	
	prevSender->unselected();
	senderButton->selected();
	prevSender = senderButton;
}

// KANA TAB CALLBACKS
void	DictionaryInterface::kanaRowsCallback(Ref* pSender)
{
	// TODO : DISABLE TOUCHES BEHIND THE PAGE

	//

	m_currentKanaPage = ((MenuItemFont*)pSender)->getName();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto appDictionary = AppDictionary::getInstance();

	Sprite* underLayer = Sprite::create("textures/interface/popup/popupmenu_background.png",
		Rect(0, 0, visibleSize.width, visibleSize.height));
	underLayer->setAnchorPoint(Vec2(0, 0));
	underLayer->setPosition(Vec2(0, 0));

	Layer* kanaPageLayer = Layer::create();

	Sprite* kanaPage = Sprite::create("textures/kanapages/background.jpg");
	kanaPage->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	kanaPageLayer->addChild(kanaPage, SHOW_PAGE_BACKGROUND_ORDER, SHOW_PAGE_BACKGROUND);
	
	std::string kanaTypeFolder;

	if (m_currentKanaType == DICTIONARY_KANA_TYPE_HIRAGANA)
		kanaTypeFolder = "hiragana";
	else
		kanaTypeFolder = "katakana";

	m_currentKanaSpriteInitFilename = "animations/kanatips/" + kanaTypeFolder + "/" + m_currentKanaPage + "/ (1).jpg";
	auto kanaSprite = Sprite::create(m_currentKanaSpriteInitFilename);
	auto kanaSpritePosition = Point(kanaPage->getPosition().x - kanaPage->getContentSize().width / 5 + 5,
		kanaPage->getPosition().y + kanaPage->getContentSize().height / 4);
	kanaSprite->setPosition(kanaSpritePosition);
	kanaPageLayer->addChild(kanaSprite, SHOW_PAGE_SPRITE_ORDER, SHOW_PAGE_SPRITE);

	m_canvas = Canvas::create();
	m_canvas->setPosition(Vec2(kanaPage->getPosition().x - kanaPage->getContentSize().width / 2 + 20,
		kanaPage->getPosition().y - kanaPage->getContentSize().height / 2 + 70));
	m_canvas->setBrushDepth(m_kanaPageBrushDepth);
	kanaPageLayer->addChild(m_canvas, SHOW_PAGE_SPRITE_ORDER);

	// <KANA_PAGE_MENU>
	auto clearCanvasButton = MenuItemImage::create("textures/kanapages/menu_button.png",
		"textures/kanapages/menu_button.png",
		CC_CALLBACK_1(DictionaryInterface::clearCanvasButtonCallback, this));
	clearCanvasButton->setPosition(Vec2(kanaPage->getPosition().x + kanaPage->getContentSize().width / 2 + 125,
		kanaPage->getPosition().y - kanaPage->getContentSize().height / 2 + 20));

	auto setOutlineButton = MenuItemImage::create("textures/kanapages/menu_button.png",
		"textures/kanapages/menu_button.png",
		CC_CALLBACK_1(DictionaryInterface::setOutlineButtonCallback, this));
	setOutlineButton->setPosition(Vec2(clearCanvasButton->getPosition().x,
		clearCanvasButton->getPosition().y + clearCanvasButton->getContentSize().height + KANA_PAGE_BUTTON_VERTICAL_OFFSET));

	auto brushScaleDownButton = MenuItemImage::create("textures/kanapages/brush_scaledown_button.png",
		"textures/kanapages/brush_scaledown_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::brushScaleButtonCallback, this));
	brushScaleDownButton->setPosition(Vec2(setOutlineButton->getPosition().x - setOutlineButton->getContentSize().width / 2
		+ brushScaleDownButton->getContentSize().width - 16,
		setOutlineButton->getPosition().y + setOutlineButton->getContentSize().height + KANA_PAGE_BUTTON_VERTICAL_OFFSET));
	brushScaleDownButton->setTag(KANA_PAGE_BRUSH_SCALEDOWN);

	auto brushScaleUpButton = MenuItemImage::create("textures/kanapages/brush_scaleup_button.png",
		"textures/kanapages/brush_scaleup_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::brushScaleButtonCallback, this));
	brushScaleUpButton->setPosition(Vec2(brushScaleDownButton->getPosition().x,
		brushScaleDownButton->getPosition().y + brushScaleDownButton->getContentSize().height + KANA_PAGE_BUTTON_VERTICAL_OFFSET));
	brushScaleUpButton->setTag(KANA_PAGE_BRUSH_SCALEUP);

	auto closeButton = MenuItemImage::create("textures/kanapages/close_button.png",
		"textures/kanapages/close_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::closeButtonCallback, this));
	closeButton->setPosition(Vec2(kanaPage->getPosition().x + kanaPage->getContentSize().width / 2 - 2 * closeButton->getContentSize().width / 3,
		kanaPage->getPosition().y + kanaPage->getContentSize().height / 2 - 2 * closeButton->getContentSize().height / 3));

	auto kanaPageMenu = Menu::create(clearCanvasButton, setOutlineButton, 
		brushScaleUpButton, brushScaleDownButton, closeButton, nullptr);
	kanaPageMenu->setPosition(Vec2(0, 0));
	kanaPageLayer->addChild(kanaPageMenu, SHOW_PAGE_HINT_ORDER);
	// </KANA_PAGE_MENU>

	// <KANA_SYMBOL_ANIMATION>
	Vector<SpriteFrame*> kanaSpriteFrames;
	for (int i = KANA_ANIMATION_MAX_FRAMES; i >= 1; i--)
	{
		SpriteFrame* newFrame = SpriteFrame::create("animations/kanatips/" + kanaTypeFolder + "/" + m_currentKanaPage + "/ (" + std::to_string(i) + ").jpg",
			Rect(0, 0, 238, 238));

		if (newFrame->getTexture())
			kanaSpriteFrames.pushBack(newFrame);
	}
	
	Animation* animation = Animation::createWithSpriteFrames(kanaSpriteFrames, 0.1f);
	Animate* animate = Animate::create(animation);
	kanaSprite->runAction(RepeatForever::create(animate));
	// </KANA_SYMBOL_ANIMATION>

	// <HINTS_AND_LABELS>
	PopupMenu* hint = PopupMenu::create(visibleSize.width / 2 - 1, 60, 423, 55);
	hint->setCurrentText("We understand your wish to learn fast\nbut it's better to write symbols with note");
	hint->setHeaderText(" ");

	std::string kanaKeyButtonChar = appDictionary->getKeyButtonByRomaji(m_currentKanaPage);
	Label* keyButtonLabel = Label::create("[" + kanaKeyButtonChar + "] on English\nkeyboard", "Arial", 18,
		Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	keyButtonLabel->setTextColor(Color4B(0, 0, 0, 255));
	keyButtonLabel->setPosition(Vec2(kanaSpritePosition.x + kanaSprite->getContentSize().width * 5 / 6,
		kanaSpritePosition.y + kanaSprite->getContentSize().height * 5 / 12));

	Label* clearCanvasButtonLabel = Label::create("Clear canvas", "fonts/Xerox Sans Serif Wide Oblique.ttf", 17,
		Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	Label* setOutlineButtonLabel = Label::create("Enable/Disable outline", "fonts/Xerox Sans Serif Wide Oblique.ttf", 17,
		Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	Label* brushDepthPXLabel = Label::create("Brush depth: " + std::to_string((int)m_canvas->getBrushDepth()),
		"fonts/Xerox Sans Serif Wide Oblique.ttf", 17, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	clearCanvasButtonLabel->setPosition(clearCanvasButton->getPosition());
	setOutlineButtonLabel->setPosition(setOutlineButton->getPosition());
	brushDepthPXLabel->setPosition(setOutlineButton->getPosition().x,
		brushScaleDownButton->getPosition().y + KANA_PAGE_BUTTON_VERTICAL_OFFSET * 2);
	// </HINTS_AND_LABELS>

	kanaPageLayer->addChild(clearCanvasButtonLabel, SHOW_PAGE_MENU_LABEL_ORDER);
	kanaPageLayer->addChild(setOutlineButtonLabel, SHOW_PAGE_MENU_LABEL_ORDER);
	kanaPageLayer->addChild(keyButtonLabel, SHOW_PAGE_HINT_ORDER);
	kanaPageLayer->addChild(brushDepthPXLabel, SHOW_PAGE_HINT_ORDER, KANA_PAGE_BRUSH_DEPTH_HINT);

	this->addChild(underLayer, DICTIONARY_SHOW_PAGE_UNDERLAYER_ORDER, DICTIONARY_SHOW_PAGE_UNDERLAYER);
	this->addChild(kanaPageLayer, DICTIONARY_SHOW_PAGE_LAYER_ORDER, DICTIONARY_SHOW_PAGE_LAYER);
	this->addChild(hint, DICTIONARY_SHOW_PAGE_HINT_ORDER, DICTIONARY_SHOW_PAGE_HINT);

	underLayer->setOpacity(0);
	underLayer->runAction(FadeIn::create(0.5f));

	// <LAUNCH_SCHEDULE_FOR_EXAMPLES>
	this->schedule(schedule_selector(DictionaryInterface::onEachFrameExampleView), 0.1f);
	// </LAUNCH_SCHEDULE_FOR_EXAMPLES>
}

void	DictionaryInterface::closeButtonCallback(Ref* pSender)
{
	Sprite* underLayer = (Sprite*)this->getChildByTag(DICTIONARY_SHOW_PAGE_UNDERLAYER);
	Layer* kanaPageLayer = (Layer*)this->getChildByTag(DICTIONARY_SHOW_PAGE_LAYER);
	PopupMenu* hint = (PopupMenu*)this->getChildByTag(DICTIONARY_SHOW_PAGE_HINT);

	underLayer->runAction(Sequence::create(FadeOut::create(0.5f),
		CallFunc::create(CC_CALLBACK_0(DictionaryInterface::_underlayerAfterFadeOut, this)), nullptr));
	kanaPageLayer->removeFromParentAndCleanup(true);

	this->unschedule(schedule_selector(DictionaryInterface::onEachFrameExampleView));

	if (hint != nullptr)
	{
		hint->closePopup();
	}
}

void	DictionaryInterface::clearCanvasButtonCallback(Ref* pSender)
{
	m_canvas->resetDynamicTexture();
}

void	DictionaryInterface::setOutlineButtonCallback(Ref* pSender)
{
	Layer* kanaPageLayer = (Layer*) this->getChildByTag(DICTIONARY_SHOW_PAGE_LAYER);

	Sprite* kanaOutline = (Sprite*) kanaPageLayer->getChildByTag(KANA_PAGE_OUTLINE);
	if (kanaOutline != nullptr)
	{
		kanaOutline->removeFromParentAndCleanup(true);
		return;
	}

	Sprite* kanaSprite = (Sprite*) kanaPageLayer->getChildByTag(SHOW_PAGE_SPRITE);
	kanaOutline = Sprite::create(m_currentKanaSpriteInitFilename);

	kanaOutline->setPosition(Vec2(m_canvas->getPosition().x + m_canvas->getContentSize().width / 2,
		m_canvas->getPosition().y + m_canvas->getContentSize().height / 2));
	kanaOutline->setOpacity(64);
	kanaPageLayer->addChild(kanaOutline, SHOW_PAGE_HINT_ORDER, KANA_PAGE_OUTLINE);
}

void	DictionaryInterface::brushScaleButtonCallback(Ref* pSender)
{
	auto scaleTag = ((MenuItem*)pSender)->getTag();
	auto brushPoint = m_canvas->getBrushPoint();
	auto brushSize = brushPoint->getContentSize();

	switch (scaleTag)
	{
	case KANA_PAGE_BRUSH_SCALEUP:
		m_kanaPageBrushDepth = m_canvas->getBrushDepth() + 1;
		break;
	case KANA_PAGE_BRUSH_SCALEDOWN:
		m_kanaPageBrushDepth = m_canvas->getBrushDepth() - 1;
		break;
	}
	m_canvas->setBrushDepth(m_kanaPageBrushDepth);

	Layer* kanaPageLayer = (Layer*) this->getChildByTag(DICTIONARY_SHOW_PAGE_LAYER);
	Label* brushDepthLabel = (Label*)kanaPageLayer->getChildByTag(KANA_PAGE_BRUSH_DEPTH_HINT);

	brushDepthLabel->setString("Brush depth: " + std::to_string((int)m_canvas->getBrushDepth()));
}

void	DictionaryInterface::onEachFrameExampleView(float dt)
{
	static long millisecondsCounter = 0;
	static short exampleNum = 1;

	if (millisecondsCounter == 0 || millisecondsCounter % 50 == 0)
	{
		auto kanaPageLayer = this->getChildByTag(DICTIONARY_SHOW_PAGE_LAYER);
		
		PopupMenu* exampleImage = (PopupMenu*)kanaPageLayer->getChildByTag(KANA_PAGE_EXAMPLE);
		if (exampleImage != nullptr)
			exampleImage->closePopup();

		auto visibleSize = Director::getInstance()->getVisibleSize();
		std::string kanaTypeFolder;
		if (m_currentKanaType == DICTIONARY_KANA_TYPE_HIRAGANA)
			kanaTypeFolder = "hiragana";
		else
			kanaTypeFolder = "katakana";

		exampleImage = PopupMenu::create("textures/kanapages/" + kanaTypeFolder + "/" + m_currentKanaPage + "/" + std::to_string(exampleNum) + ".png",
			visibleSize.width * 2 / 3 - 40, visibleSize.height / 2);

		if (exampleImage != nullptr)
		{
			kanaPageLayer->addChild(exampleImage, SHOW_PAGE_HINT_ORDER, KANA_PAGE_EXAMPLE);
			exampleImage->enableAutoClose(true);

			exampleNum++;
		}
		else
		{
			exampleNum = 1;
			millisecondsCounter = -1;
		}
	}

	millisecondsCounter++;
}

// KANJI TAB CALLBACKS
void	DictionaryInterface::switchKanjiInputTypeCallback(Ref* pSender)
{
	short prevKanjiInputType = m_currentKanjiInputType;
	MenuItemImage* senderButton = (MenuItemImage*)pSender;
	MenuItemImage* prevSender = (MenuItemImage*)senderButton->getParent()->getChildByTag(prevKanjiInputType);
	int senderTag = senderButton->getTag();

	m_currentKanjiInputType = senderTag;
	
	if (m_currentKanjiInputType != prevKanjiInputType)
	{
		Layer* searchResultLayer = (Layer*)m_tabLayer->getChildByTag(KANJI_TAB_SEARCH_RESULT_LAYER);
		if (searchResultLayer != nullptr)
			searchResultLayer->retain();

		m_tabLayer->removeAllChildrenWithCleanup(true);

		if (searchResultLayer != nullptr)
		{
			m_tabLayer->addChild(searchResultLayer, KANJI_TAB_RESULT_LAYER_ORDER, KANJI_TAB_SEARCH_RESULT_LAYER);
			searchResultLayer->release();
		}
	}
	else
		return;

	switch (senderTag)
	{
	case DICTIONARY_KANJI_INPUT_TYPE_IMAGE:
		_createKanjiTabImageSearch();
		break;
	case DICTIONARY_KANJI_INPUT_TYPE_TEXT:
		_createKanjiTabTextSearch();
		break;
	}

	prevSender = senderButton;
}

void	DictionaryInterface::kanjiEnterButtonCallback(Ref* pSender)
{
	auto resultLayer = this->getChildByTag(DICTIONARY_TAB_LAYER)->getChildByTag(DICTIONARY_SEARCH_RESULT);

	if (resultLayer != nullptr)
	{
		resultLayer->removeFromParentAndCleanup(true);
	}

	this->_createKanjiResults();

	// WARNING!!! ONLY FOR TEACHING!!!
	//
	// this->teachKanjiSystemSinceID(21);
	//
	//
}

void	DictionaryInterface::searchResultButtonCallback(Ref* pSender)
{
	MenuItemImage* senderItem = (MenuItemImage*)pSender;
	int kanjiID = senderItem->getTag();
	auto kanjiDict = AppDictionary::getInstance();
	auto dictionaryPageTwo = this->getChildByTag(DICTIONARY_PAGE_TWO);
	Layer* searchResultLayer = (Layer*)m_tabLayer->getChildByTag(KANJI_TAB_SEARCH_RESULT_LAYER);

	// <CLEAR_OLD_INFO>
	if (searchResultLayer != nullptr)
	{
		searchResultLayer->removeAllChildrenWithCleanup(true);
		searchResultLayer->removeFromParentAndCleanup(true);
	}
	
	searchResultLayer = Layer::create();
	// </CLEAR_OLD_INFO>

	auto kanjiPosition = Vec2(dictionaryPageTwo->getPosition().x + 65,
		dictionaryPageTwo->getPosition().y + dictionaryPageTwo->getContentSize().height - 65);
	auto kanjiSize = senderItem->getNormalImage()->getContentSize();;

	std::string imageFilename = "textures/kanji/" + std::to_string(kanjiID) + ".png";

	Sprite* kanjiSprite = Sprite::create(imageFilename);
	kanjiSprite->setPosition(kanjiPosition);

	Kanji* foundedKanji = kanjiDict->getKanjiByID(kanjiID);
	std::string formattedString = foundedKanji->getFormattedDescription();

	auto lReadingKanji = Label::create(formattedString, "Arial", 16);
	lReadingKanji->setAnchorPoint(Vec2(0, 1));
	lReadingKanji->setTextColor(Color4B(0, 0, 0, 255));
	lReadingKanji->setPosition(Vec2(kanjiPosition.x - kanjiSize.width / 2,
		kanjiPosition.y - kanjiSize.height / 2 - 10));

	searchResultLayer->addChild(kanjiSprite, KANJI_TAB_SPRITE_ORDER);
	searchResultLayer->addChild(lReadingKanji, KANJI_TAB_HINT_ORDER);

	m_tabLayer->addChild(searchResultLayer, KANJI_TAB_RESULT_LAYER_ORDER, KANJI_TAB_SEARCH_RESULT_LAYER);
}

// WORDS TAB CALLBACKS
void	DictionaryInterface::wordEnterButtonCallback(Ref* pSender)
{
	auto tabChildren = m_tabLayer->getChildren();
	for (cocos2d::Vector<Node*>::iterator it = tabChildren.begin(); it != tabChildren.end(); it++)
	{
		if ((*it)->getTag() == DICTIONARY_SEARCH_RESULT)
		{
			(*it)->removeFromParentAndCleanup(true);
		}
	}

	ui::TextField* textField = (ui::TextField*) m_tabLayer->getChildByTag(DICTIONARY_TEXTFIELD);
	std::string word = textField->getString();
	auto appDictionary = AppDictionary::getInstance();

	std::vector<DictionaryWord*> foundedWords = appDictionary->getWordsByKana(word);

	int x = 380;
	int y = 330;

	for (int i = 0; i < foundedWords.size(); i++)
	{
		auto formattedWord = foundedWords[i]->getKanjiWriting() + " (" + foundedWords[i]->getKanaWriting() + ") "
			+ "[" + foundedWords[i]->getRomajiWriting() + "]\n-> ";
		std::vector<std::string> translations = foundedWords[i]->getTranslations();

		int k = 1;
		for (std::vector<std::string> ::iterator it = translations.begin(); it != translations.end(); it++)
		{
			if (translations.size() > 1)
				formattedWord += std::to_string(k) + ". ";

			formattedWord += *it + "; ";
			
			k++;
		}
		
		auto wordLabel = Label::create(formattedWord, "Arial", 16, Size::ZERO, TextHAlignment::LEFT, TextVAlignment::CENTER);
		wordLabel->setAnchorPoint(Vec2(0, 1));
		wordLabel ->setPosition(Vec2(x, y));
		wordLabel->setTextColor(Color4B(0, 0, 0, 255));
		m_tabLayer->addChild(wordLabel, 1, DICTIONARY_SEARCH_RESULT);

		y -= 20;

		auto splitterLabel = Label::create("_________________________________", "Arial", 16, Size::ZERO, TextHAlignment::LEFT, TextVAlignment::CENTER);
		splitterLabel->setAnchorPoint(Vec2(0, 1));
		splitterLabel->setPosition(Vec2(x, y));
		splitterLabel->setTextColor(Color4B(0, 0, 0, 255));
		m_tabLayer->addChild(splitterLabel, 1, DICTIONARY_SEARCH_RESULT);

		y -= 20;
	}
}

// GRAMMAR TAB CALLBACKS
void	DictionaryInterface::grammarItemCallback(Ref* pSender)
{
	// TODO
	ID grammarID = ((MenuItemFont*)pSender)->getTag();
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto appDictionary = AppDictionary::getInstance();

	Sprite* underLayer = Sprite::create("textures/interface/popup/popupmenu_background.png",
		Rect(0, 0, visibleSize.width, visibleSize.height));
	underLayer->setAnchorPoint(Vec2(0, 0));
	underLayer->setPosition(Vec2(0, 0));

	Layer* grammarPageLayer = Layer::create();

	Sprite* grammarPage = Sprite::create("textures/kanapages/background.jpg");
	grammarPage->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	grammarPageLayer->addChild(grammarPage, SHOW_PAGE_BACKGROUND_ORDER, SHOW_PAGE_BACKGROUND);

	// <GRAMMAR_TEXT>
	std::string formattedGrammarText;

	std::vector<std::string> grammarTextVector = appDictionary->getGrammarTextByID(grammarID);
	for (std::vector<std::string>::iterator line = grammarTextVector.begin();
		line != grammarTextVector.end(); line++)
	{
		formattedGrammarText += *line + '\n';
	}

	Label* grammarText = Label::create(formattedGrammarText, "Arial", 18);
	grammarText->setColor(Color3B(0, 0, 0));
	grammarText->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	grammarPageLayer->addChild(grammarText, SHOW_PAGE_HINT_ORDER);
	// </GRAMMAR_TEXT>

	// <INIT_MENU>
	auto closeButton = MenuItemImage::create("textures/kanapages/close_button.png",
		"textures/kanapages/close_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::closeButtonCallback, this));
	closeButton->setPosition(Vec2(grammarPage->getPosition().x + grammarPage->getContentSize().width / 2 - 2 * closeButton->getContentSize().width / 3,
		grammarPage->getPosition().y + grammarPage->getContentSize().height / 2 - 2 * closeButton->getContentSize().height / 3));

	auto grammarPageMenu = Menu::create(closeButton, nullptr);
	grammarPageMenu->setPosition(Vec2(0, 0));
	grammarPageLayer->addChild(grammarPageMenu, SHOW_PAGE_HINT_ORDER);
	// </INIT_MENU>

	this->addChild(underLayer, DICTIONARY_SHOW_PAGE_UNDERLAYER_ORDER, DICTIONARY_SHOW_PAGE_UNDERLAYER);
	this->addChild(grammarPageLayer, DICTIONARY_SHOW_PAGE_LAYER_ORDER, DICTIONARY_SHOW_PAGE_LAYER);
	
	underLayer->setOpacity(0);
	underLayer->runAction(FadeIn::create(0.5f));
}

// OTHER CALLBACKS
void	DictionaryInterface::helpButtonCallback(Ref* pSender)
{
	int helperTag = ((MenuItem*)pSender)->getTag();

	PopupMenu* helper = (PopupMenu*)this->getChildByTag(DICTIONARY_HINT);
	if (helper == nullptr)
	{
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Vec2 helperPosition = Vec2(c_alignOffset + 200, visibleSize.height - 133 - c_alignOffset);
		std::string helperText;

		switch (helperTag)
		{
		case DICIONARY_KANJI_HELPER:
			helper = PopupMenu::create(helperPosition.x, helperPosition.y, 400, 133);
			helperText = "1. Choose input type\n\n";
			helperText += "2. Draw or enter your kanji inside the box\n\n";
			helperText += "3. If your type is \"Image\" then try to draw\nkanji using all the space of canvas";
			helper->setCurrentText(helperText);
			helper->setHeaderText("\n");
			break;
		case DICIONARY_WORDS_HELPER:
			helper = PopupMenu::create("textures/interface/dictionary/jap_keyboard_helper.png", 0, 0);
			helper->setPosition(Vec2(helperPosition.x, helperPosition.y));
			helper->setHeaderText("Keyboard helper");
			break;
		}

		this->addChild(helper, DICTIONARY_HINT_ORDER, DICTIONARY_HINT);
	}
	else
	{
		helper->closePopup();
	}
}
// </CALLBACKS>

// <SUBFUNCTIONS>
void	DictionaryInterface::_createKanjiTabImageSearch()
{
	// <CANVAS>
	auto dictionaryPageOne = this->getChildByTag(DICTIONARY_PAGE_ONE);

	m_canvas = Canvas::create();
	m_canvas->Canvas::setPosition(Vec2(dictionaryPageOne->getContentSize().width / 2 - m_canvas->getContentSize().width / 2 + c_alignOffset,
		dictionaryPageOne->getContentSize().height - m_canvas->getContentSize().height));
	m_canvas->setBrushDepth(MAX_BRUSH_DEPTH_PX - 4);
	// </CANVAS>

	// <MENU>
	auto imageTypeButton = MenuItemImage::create("textures/interface/dictionary/side_button.png",
		"textures/interface/dictionary/side_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::switchKanjiInputTypeCallback, this));
	auto textTypeButton = MenuItemImage::create("textures/interface/dictionary/side_button.png",
		"textures/interface/dictionary/side_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::switchKanjiInputTypeCallback, this));

	auto helpButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::helpButtonCallback, this));
	auto enterButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanjiEnterButtonCallback, this));
	auto clearButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::clearCanvasButtonCallback, this));

	imageTypeButton->setPosition(Vec2(135, 225));
	textTypeButton->setPosition(Vec2(135, 105));

	helpButton->setPosition(Vec2(-115, 0));
	enterButton->setPosition(Vec2(-5, 0));
	clearButton->setPosition(Vec2(105, 0));

	imageTypeButton->setTag(DICTIONARY_KANJI_INPUT_TYPE_IMAGE);
	textTypeButton->setTag(DICTIONARY_KANJI_INPUT_TYPE_TEXT);
	helpButton->setTag(DICIONARY_KANJI_HELPER);

	imageTypeButton->selected();

	Menu* kanjiInputMenu = Menu::create(helpButton, enterButton, clearButton, imageTypeButton, textTypeButton, nullptr);
	auto kanjiInputMenuPosition = Vec2(c_splitLinePosition.x - 10, c_splitLinePosition.y - 50);
	kanjiInputMenu->setPosition(kanjiInputMenuPosition);

	auto lImageTypeButton = Label::create("Image", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lTextTypeButton = Label::create("Text", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	auto lHelpButton = Label::create("Help me!", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lEnterButton = Label::create("Search", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lClearButton = Label::create("Clear", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	lImageTypeButton->setPosition(Vec2(kanjiInputMenuPosition.x + imageTypeButton->getPosition().x,
		kanjiInputMenuPosition.y + imageTypeButton->getPosition().y));
	lTextTypeButton->setPosition(Vec2(kanjiInputMenuPosition.x + textTypeButton->getPosition().x,
		kanjiInputMenuPosition.y + textTypeButton->getPosition().y));
	lImageTypeButton->setRotation(90.0f);
	lTextTypeButton->setRotation(90.0f);

	lHelpButton->setPosition(Vec2(kanjiInputMenuPosition.x + helpButton->getPosition().x,
		kanjiInputMenuPosition.y + helpButton->getPosition().y));
	lEnterButton->setPosition(Vec2(kanjiInputMenuPosition.x + enterButton->getPosition().x,
		kanjiInputMenuPosition.y + enterButton->getPosition().y));
	lClearButton->setPosition(Vec2(kanjiInputMenuPosition.x + clearButton->getPosition().x,
		kanjiInputMenuPosition.y + clearButton->getPosition().y));

	auto lInputType = Label::create("Input type:", "fonts/Xerox Serif Wide.ttf", 20, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	lInputType->setTextColor(Color4B(0, 0, 0, 255));
	lInputType->setPosition(Vec2(c_splitLinePosition.x - 155, c_splitLinePosition.y + 125));
	lInputType->setRotation(-90.0f);
	// </MENU>

	m_tabLayer->addChild(m_canvas, 1);
	m_tabLayer->addChild(kanjiInputMenu, 2);
	m_tabLayer->addChild(lInputType, 3);
	m_tabLayer->addChild(lImageTypeButton, 3);
	m_tabLayer->addChild(lTextTypeButton, 3);
	m_tabLayer->addChild(lHelpButton, 3);
	m_tabLayer->addChild(lEnterButton, 3);
	m_tabLayer->addChild(lClearButton, 3);
}

void	DictionaryInterface::_createKanjiTabTextSearch()
{
	// <TEXTFIELD>
	auto textField = ui::TextField::create("Enter the word here...", "Arial", 20);

	textField->setPosition(c_splitLinePosition);
	textField->setMaxLength(c_stringMaxLength);
	textField->setMaxLengthEnabled(true);
	textField->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
		CCLOG("Editing a TextField...");
	});

	m_tabLayer->addChild(textField, 1, DICTIONARY_TEXTFIELD);
	// </TEXTFIELD>

	// <MENU>
	auto imageTypeButton = MenuItemImage::create("textures/interface/dictionary/side_button.png",
		"textures/interface/dictionary/side_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::switchKanjiInputTypeCallback, this));
	auto textTypeButton = MenuItemImage::create("textures/interface/dictionary/side_button.png",
		"textures/interface/dictionary/side_button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::switchKanjiInputTypeCallback, this));

	auto helpButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::helpButtonCallback, this));
	auto enterButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanjiEnterButtonCallback, this));
	
	imageTypeButton->setPosition(Vec2(135, 225));
	textTypeButton->setPosition(Vec2(135, 105));

	helpButton->setPosition(Vec2(-60, 0));
	enterButton->setPosition(Vec2(50, 0));

	imageTypeButton->setTag(DICTIONARY_KANJI_INPUT_TYPE_IMAGE);
	textTypeButton->setTag(DICTIONARY_KANJI_INPUT_TYPE_TEXT);
	helpButton->setTag(DICIONARY_KANJI_HELPER);

	textTypeButton->selected();

	Menu* kanjiInputMenu = Menu::create(helpButton, enterButton, imageTypeButton, textTypeButton, nullptr);
	auto kanjiInputMenuPosition = Vec2(c_splitLinePosition.x - 10, c_splitLinePosition.y - 50);
	kanjiInputMenu->setPosition(kanjiInputMenuPosition);

	auto lImageTypeButton = Label::create("Image", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lTextTypeButton = Label::create("Text", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	auto lHelpButton = Label::create("Help me!", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lEnterButton = Label::create("Search", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	
	lImageTypeButton->setPosition(Vec2(kanjiInputMenuPosition.x + imageTypeButton->getPosition().x,
		kanjiInputMenuPosition.y + imageTypeButton->getPosition().y));
	lTextTypeButton->setPosition(Vec2(kanjiInputMenuPosition.x + textTypeButton->getPosition().x,
		kanjiInputMenuPosition.y + textTypeButton->getPosition().y));
	lImageTypeButton->setRotation(90.0f);
	lTextTypeButton->setRotation(90.0f);

	lHelpButton->setPosition(Vec2(kanjiInputMenuPosition.x + helpButton->getPosition().x,
		kanjiInputMenuPosition.y + helpButton->getPosition().y));
	lEnterButton->setPosition(Vec2(kanjiInputMenuPosition.x + enterButton->getPosition().x,
		kanjiInputMenuPosition.y + enterButton->getPosition().y));
	
	auto lInputType = Label::create("Input type:", "fonts/Xerox Serif Wide.ttf", 20, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	lInputType->setTextColor(Color4B(0, 0, 0, 255));
	lInputType->setPosition(Vec2(c_splitLinePosition.x + 85, c_splitLinePosition.y + 125));
	lInputType->setRotation(-90.0f);
	// </MENU>

	m_tabLayer->addChild(kanjiInputMenu, 2);
	m_tabLayer->addChild(lInputType, 3);
	m_tabLayer->addChild(lImageTypeButton, 3);
	m_tabLayer->addChild(lTextTypeButton, 3);
	m_tabLayer->addChild(lHelpButton, 3);
	m_tabLayer->addChild(lEnterButton, 3);
}

void	DictionaryInterface::_createKanjiResults()
{
	// <CLEAR_OLD_INFO>
	Layer* searchResultLayer = (Layer*)m_tabLayer->getChildByTag(KANJI_TAB_SEARCH_RESULT_LAYER);

	if (searchResultLayer != nullptr)
	{
		searchResultLayer->removeAllChildrenWithCleanup(true);
		searchResultLayer->removeFromParentAndCleanup(true);
	}
	
	searchResultLayer = Layer::create();
	// </CLEAR_OLD_INFO>

	// <FIND_KANJI>
	AppDictionary* dict = AppDictionary::getInstance();
	std::vector<Kanji*> searchResult;
	if (m_currentKanjiInputType == DICTIONARY_KANJI_INPUT_TYPE_IMAGE)
	{
		Vec2 canvasPosition = m_canvas->getPosition();
		Size canvasSize = m_canvas->getContentSize();
		Rect canvasBoundingBox = Rect(canvasPosition.x, canvasPosition.y,
			canvasSize.width, canvasSize.height);

		glAddition::captureRectToTGA(canvasBoundingBox, "to_analyze.tga");
		searchResult = dict->getKanjiByImage(canvasBoundingBox);
	}
	else
	{
		auto textField = (ui::TextField*)m_tabLayer->getChildByTag(DICTIONARY_TEXTFIELD);
		searchResult = dict->getKanjiByRomaji(textField->getString());
	}
	// </FIND_KANJI>

	// <LAYOUT>
	auto kanjiDict = AppDictionary::getInstance();
	auto dictionaryPageTwo = this->getChildByTag(DICTIONARY_PAGE_TWO);
	
	int horizontalOffset = 0, verticalOffset = 0;
	cocos2d::Vector<MenuItem*> buttonVector;
	for (std::vector<Kanji*>::iterator it = searchResult.begin(); it != searchResult.end(); it++)
	{
		// <CREATE_KANJI_RESULT_BUTTON>
		std::string imageFilename = "textures/kanji/" + std::to_string((*it)->getID()) + ".png";
		
		auto kanjiButton = MenuItemImage::create(imageFilename, imageFilename,
			CC_CALLBACK_1(DictionaryInterface::searchResultButtonCallback, this));
		kanjiButton->setTag((*it)->getID());

		auto kanjiPosition = Vec2(dictionaryPageTwo->getPosition().x + 65 + horizontalOffset,
			dictionaryPageTwo->getPosition().y + dictionaryPageTwo->getContentSize().height - 65 + verticalOffset);
		static auto kanjiSize = kanjiButton->getNormalImage()->getContentSize();
		kanjiButton->setPosition(kanjiPosition);

		buttonVector.pushBack(kanjiButton);
		// </CREATE_KANJI_RESULT_BUTTON>

		// <DRAW_BUTTON_BORDER>
		auto rectWithBorder = DrawNode::create();
		Vec2 vertices[] =
		{
			Vec2(kanjiPosition.x - kanjiSize.width / 2, kanjiPosition.y - kanjiSize.height / 2),
			Vec2(kanjiPosition.x - kanjiSize.width / 2, kanjiPosition.y + kanjiSize.height / 2),
			Vec2(kanjiPosition.x + kanjiSize.width / 2,	kanjiPosition.y + kanjiSize.height / 2),
			Vec2(kanjiPosition.x + kanjiSize.width / 2, kanjiPosition.y - kanjiSize.height / 2),
		};
		rectWithBorder->drawPolygon(vertices, 4, Color4F(1.0f, 1.0f, 1.0f, 0), 2, Color4F(0.2f, 0.2f, 0.2f, 1));

		searchResultLayer->addChild(rectWithBorder, KANJI_TAB_SPRITE_BORDER_ORDER);
		// </DRAW_BUTTON_BORDER>

		horizontalOffset += kanjiSize.width + 10;
		if (horizontalOffset >= kanjiSize.width * 3)
		{
			verticalOffset -= kanjiSize.width + 10;
			horizontalOffset = 0;
		}

		if (searchResultLayer->getChildrenCount() >= KANJI_SEARCH_MAX_RESULT)
			break;
	}

	Menu* searchResultMenu = Menu::createWithArray(buttonVector);
	searchResultMenu->setPosition(0, 0); // dictionaryPageTwo->getPosition());

	searchResultLayer->addChild(searchResultMenu, KANJI_TAB_SPRITE_ORDER);

	m_tabLayer->addChild(searchResultLayer, KANJI_TAB_RESULT_LAYER_ORDER, KANJI_TAB_SEARCH_RESULT_LAYER);
	// </LAYOUT>
}

void	DictionaryInterface::_createWordsTab()
{
	// <TEXTFIELD>
	auto textField = ui::TextField::create("Enter the word here...", "Arial", 20);

	textField->setPosition(c_splitLinePosition);
	textField->setMaxLength(c_stringMaxLength);
	textField->setMaxLengthEnabled(true);
	textField->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type){
		CCLOG("Editing a TextField...");
	});

	this->schedule(schedule_selector(DictionaryInterface::onEachFrameWordTab));

	m_tabLayer->addChild(textField, 1, DICTIONARY_TEXTFIELD);
	// </TEXTFIELD>

	// <SWITCH_KANA_TYPE_MENU>
	auto typeHiragana = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanaTypeMenuCallback, this));
	auto typeKatakana = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanaTypeMenuCallback, this));
	auto helpButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::helpButtonCallback, this));
	auto enterButton = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::wordEnterButtonCallback, this));

	typeHiragana->setTag(DICTIONARY_KANA_TYPE_HIRAGANA);
	typeKatakana->setTag(DICTIONARY_KANA_TYPE_KATAKANA);

	typeHiragana->setPosition(Vec2(-80, 0));
	typeKatakana->setPosition(Vec2(80, 0));
	helpButton->setPosition(Vec2(0, 150));
	enterButton->setPosition(Vec2(0, -100));

	helpButton->setTag(DICIONARY_WORDS_HELPER);

	Menu* kanaTypeMenu = Menu::create(typeHiragana, typeKatakana, helpButton, enterButton, nullptr);
	kanaTypeMenu->setPosition(Vec2(c_splitLinePosition.x - 10,
		c_splitLinePosition.y + 50));

	auto lHiragana = Label::create("Hiragana", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lKatakana = Label::create("Katakana", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lHelpButton = Label::create("Help me!", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lEnterButton = Label::create("Search", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lText = Label::create("Input type:", "Arial", 16, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);

	lHiragana->setPosition(Vec2(kanaTypeMenu->getPosition().x + typeHiragana->getPosition().x,
		kanaTypeMenu->getPosition().y + typeHiragana->getPosition().y));
	lKatakana->setPosition(Vec2(kanaTypeMenu->getPosition().x + typeKatakana->getPosition().x,
		kanaTypeMenu->getPosition().y + typeKatakana->getPosition().y));
	lHelpButton->setPosition(Vec2(kanaTypeMenu->getPosition().x + helpButton->getPosition().x,
		kanaTypeMenu->getPosition().y + helpButton->getPosition().y));
	lEnterButton->setPosition(Vec2(kanaTypeMenu->getPosition().x + enterButton->getPosition().x,
		kanaTypeMenu->getPosition().y + enterButton->getPosition().y));
	lText->setPosition(Vec2(kanaTypeMenu->getPosition().x,
		kanaTypeMenu->getPosition().y + 30));

	lText->setTextColor(Color4B(0, 0, 0, 255));

	m_currentKanaType = DICTIONARY_KANA_TYPE_HIRAGANA;
	typeHiragana->activate();

	m_tabLayer->addChild(kanaTypeMenu, 1);
	m_tabLayer->addChild(lHiragana, 2);
	m_tabLayer->addChild(lKatakana, 2);
	m_tabLayer->addChild(lHelpButton, 2, DICIONARY_WORDS_HELPER);
	m_tabLayer->addChild(lEnterButton, 2);
	m_tabLayer->addChild(lText, 2);
	// </SWITCH_KANA_TYPE_MENU>
}

void	DictionaryInterface::_createGrammarTab()
{
	// TODO : PAGINATOR
	//
	auto grammarDictionary = AppDictionary::getInstance()->getGrammarDictionary();
	
	MenuItemFont::setFontSize(18);
	cocos2d::Vector<MenuItem*> grammarMenuItems;
	for (std::map<ID, std::vector<std::string>>::iterator it = grammarDictionary.begin(); it != grammarDictionary.end(); it++)
	{
		auto newElem = MenuItemFont::create(it->second[0], CC_CALLBACK_1(DictionaryInterface::grammarItemCallback, this));
		newElem->setTag(it->first);
		grammarMenuItems.pushBack(newElem);
	}

	Menu* grammarSelection = Menu::createWithArray(grammarMenuItems); 
	grammarSelection->setPosition(Vec2(180, 330));
	grammarSelection->alignItemsVertically();

	m_tabLayer->addChild(grammarSelection, 1);
}

void	DictionaryInterface::_createKanaTab()
{
	auto kanaRowsDictionary = AppDictionary::getInstance()->getKanaRowsDictionary();
	std::vector<Menu*> menuVector;
	MenuItemFont::setFontSize(18);
	for (std::vector<std::vector<std::string>>::iterator rowIt = kanaRowsDictionary.begin(); rowIt != kanaRowsDictionary.end(); rowIt++)
	{
		cocos2d::Vector<MenuItem*> rowElems;
		for (std::vector<std::string>::iterator symIt = rowIt->begin(); symIt != rowIt->end(); symIt++)
		{
			auto newElem = MenuItemFont::create(*symIt, CC_CALLBACK_1(DictionaryInterface::kanaRowsCallback, this));
			newElem->setName(*symIt);
			rowElems.pushBack(newElem);
		}
		
		auto rowMenu = Menu::createWithArray(rowElems);
		rowMenu->alignItemsHorizontally();

		menuVector.push_back(rowMenu);
	}

	int x = 180;
	int y = 330;
	for (std::vector<Menu*>::iterator it = menuVector.begin(); it != menuVector.end(); it++)
	{
		(*it)->setPosition(Vec2(x, y));
		m_tabLayer->addChild(*it, 1);

		y -= 20;
	}

	// <KANA_TYPE_MENU>
	auto typeHiragana = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanaTypeMenuCallback, this));
	auto typeKatakana = MenuItemImage::create("textures/interface/dictionary/button.png",
		"textures/interface/dictionary/button_selected.png",
		CC_CALLBACK_1(DictionaryInterface::kanaTypeMenuCallback, this));
	
	typeHiragana->setTag(DICTIONARY_KANA_TYPE_HIRAGANA);
	typeKatakana->setTag(DICTIONARY_KANA_TYPE_KATAKANA);

	typeHiragana->setPosition(Vec2(-80, 0));
	typeKatakana->setPosition(Vec2(80, 0));
	
	Menu* kanaTypeMenu = Menu::create(typeHiragana, typeKatakana, nullptr);
	kanaTypeMenu->setPosition(Vec2(c_splitLinePosition.x - 10, c_splitLinePosition.y - 50));

	auto lHiragana = Label::create("Hiragana", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	auto lKatakana = Label::create("Katakana", "fonts/Xerox Serif Wide.ttf", 18, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER);
	
	lHiragana->setPosition(Vec2(kanaTypeMenu->getPosition().x + typeHiragana->getPosition().x,
		kanaTypeMenu->getPosition().y + typeHiragana->getPosition().y));
	lKatakana->setPosition(Vec2(kanaTypeMenu->getPosition().x + typeKatakana->getPosition().x,
		kanaTypeMenu->getPosition().y + typeKatakana->getPosition().y));
	
	m_currentKanaType = DICTIONARY_KANA_TYPE_HIRAGANA;
	typeHiragana->activate();

	m_tabLayer->addChild(kanaTypeMenu, 1);
	m_tabLayer->addChild(lHiragana, 2);
	m_tabLayer->addChild(lKatakana, 2);
	// </KANA_TYPE_MENU)>
}

void	DictionaryInterface::_underlayerAfterFadeOut()
{
	Sprite* underLayer = (Sprite*)this->getChildByTag(DICTIONARY_SHOW_PAGE_UNDERLAYER);

	underLayer->removeFromParentAndCleanup(true);
}
// </SUBFUNCTIONS>
void	DictionaryInterface::onEachFrameWordTab(float dt)
{
	static AppDictionary* appDictionary = AppDictionary::getInstance();

	ui::TextField* textField = (ui::TextField*) m_tabLayer->getChildByTag(DICTIONARY_TEXTFIELD);
	
	int stringLength = textField->getStringLength();
	static int oldStringLength = stringLength;

	if ((stringLength == oldStringLength + 1) || (stringLength != c_promptStringLength) && (oldStringLength == c_promptStringLength))
	{
		std::string newString = textField->getString();
		std::string key = newString.substr(newString.length() - 1, 1);

		std::string newSymbol;

		if (m_currentKanaType == DICTIONARY_KANA_TYPE_HIRAGANA)
			newSymbol = appDictionary->getHiraganaByLatin(key);
		else
			newSymbol = appDictionary->getKatakanaByLatin(key);
		
		m_kanaString.push_back(newSymbol);
		
		newString.clear();
		for (std::vector<std::string>::iterator it = m_kanaString.begin(); it != m_kanaString.end(); it++)
		{
			newString += *it;
		}

		textField->setString(newString);
	}
	else if ((stringLength == oldStringLength - 1) ||
		(stringLength == c_promptStringLength) && (oldStringLength == 1) &&
		(m_kanaString.size() > 0))
	{
		std::string newString;
		
		m_kanaString.pop_back();

		newString.clear();
		for (std::vector<std::string>::iterator it = m_kanaString.begin(); it != m_kanaString.end(); it++)
		{
			newString += *it;
		}

		textField->setString(newString);	
	}

	oldStringLength = stringLength;

}

// <ONLY_FOR_DEVELOPMENT>
void	DictionaryInterface::teachKanjiSystemSinceID(int startID)
{
	Vec2 canvasPosition = m_canvas->getPosition();
	Size canvasSize = m_canvas->getContentSize();
	Rect canvasBoundingBox = Rect(canvasPosition.x, canvasPosition.y,
	canvasSize.width, canvasSize.height);

	static long counter = startID;
	glAddition::savePixels(canvasBoundingBox, std::to_string(counter) + ".bin");
	counter++;
}
// </ONLY_FOR_DEVELOPMENT>