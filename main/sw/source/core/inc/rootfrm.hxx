/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef SW_ROOTFRM_HXX
#define SW_ROOTFRM_HXX

#include "layfrm.hxx"

class SwCntntFrm;
class ViewShell;
class SdrPage;
class SwFrmFmt;
class SwPaM;
class SwCursor;
class SwShellCrsr;
class SwTableCursor;
class SwLayVout;
class SwDestroyList;
class SwCurrShells;
class SwViewOption;
class SwSelectionList;
struct SwPosition;
struct SwCrsrMoveState;

#define HACK_TABLEMODE_INIT			0
#define HACK_TABLEMODE_LOCKLINES	1
#define HACK_TABLEMODE_PAINTLINES	2
#define HACK_TABLEMODE_UNLOCKLINES	3
#define HACK_TABLEMODE_EXIT			4

#define INV_SIZE	1
#define INV_PRTAREA	2
#define INV_POS		4
#define INV_TABLE	8
#define INV_SECTION	16
#define INV_LINENUM 32
#define INV_DIRECTION 64

#include <vector>

class SwRootFrm: public SwLayoutFrm
{
	//Muss das Superfluous temporaer abschalten.
	friend void AdjustSizeChgNotify( SwRootFrm *pRoot );

	//Pflegt pLastPage (Cut() und Paste() vom SwPageFrm
	friend inline void SetLastPage( SwPageFrm* );

	// Fuer das Anlegen und Zerstoeren des virtuellen Outputdevice-Managers
	friend void _FrmInit();		//erzeugt pVout
	friend void _FrmFinit();	//loescht pVout

    // PAGES01
    std::vector<SwRect> maPageRects;// returns the current rectangle for each page frame
                                    // the rectangle is extended to the top/bottom/left/right
                                    // for pages located at the outer borders
    SwRect  maPagesArea;            // the area covered by the pages
    long    mnViewWidth;            // the current page layout bases on this view width
    sal_uInt16  mnColumns;              // the current page layout bases on this number of columns
    bool    mbBookMode;             // the current page layout is in book view
    bool    mbSidebarChanged;       // the notes sidebar state has changed
    // <--

    bool    mbNeedGrammarCheck;     // true when something needs to be checked (not necessarily started yet!)

	static SwLayVout	 *pVout;
	static sal_Bool			  bInPaint;		//Schutz gegen doppelte Paints.
	static sal_Bool			  bNoVirDev; 	//Bei SystemPaints kein virt. Device

	sal_Bool	bCheckSuperfluous	:1; //Leere Seiten suchen?
	sal_Bool	bIdleFormat			:1; //Idle-Formatierer anwerfen?
	sal_Bool	bBrowseWidthValid	:1; //Ist nBrowseWidth gueltig?
	sal_Bool	bDummy2				:1; //Unbenutzt
	sal_Bool	bTurboAllowed		:1;
	sal_Bool	bAssertFlyPages		:1; //Ggf. weitere Seiten fuer Flys einfuegen?
	sal_Bool	bDummy				:1; //Unbenutzt
	sal_Bool	bIsVirtPageNum		:1;	//gibt es eine virtuelle Seitennummer ?
	sal_Bool 	bIsNewLayout		:1;	//Layout geladen oder neu erzeugt.
	sal_Bool	bCallbackActionEnabled:1; //Keine Action in Benachrichtung erwuenscht
									//siehe dcontact.cxx, ::Changed()

    //Fuer den BrowseMode. nBrowseWidth ist die Aeussere Kante des am weitesten
	//rechts stehenden Objektes. Die rechte Kante der Seiten soll im BrowseMode
	//nicht kleiner werden als dieser Wert.
	long    nBrowseWidth;

	//Wenn nur _ein: CntntFrm zu formatieren ist, so steht dieser in pTurbo.
	const SwCntntFrm *pTurbo;

	//Die letzte Seite wollen wir uns nicht immer muehsam zusammensuchen.
	SwPageFrm *pLastPage;

	//Die Root kuemmert sich nun auch um den Shell-Zugriff. Ueber das Dokument
	//sollte man auch immer an die Root herankommen und somit auch immer
	//einen Zugriff auf die Shell haben.
	//Der Pointer pCurrShell ist der Pointer auf irgendeine der Shells fuer
	//das Dokument
	//Da es durchaus nicht immer egal ist, auf welcher Shell gearbeitet wird,
	//ist es notwendig die aktive Shell zu kennen. Das wird dadurch angenaehert,
	//dass der Pointer pCurrShell immer dann umgesetzt wird, wenn eine
	//Shell den Fokus erhaelt (FEShell). Zusaetzlich wird der Pointer
	//Temporaer von SwCurrShell umgesetzt, dieses wird typischerweise
	//ueber das Macro SET_CURR_SHELL erledigt. Makro + Klasse sind in der
	//ViewShell zu finden. Diese Objekte koennen auch verschachtelt (auch fuer
	//unterschiedliche Shells) erzeugt werden. Sie werden im Array pCurrShells
	//gesammelt.
	//Weiterhin kann es noch vorkommen, dass eine Shell aktiviert wird,
	//waehrend noch ein CurrShell-Objekt "aktiv" ist. Dieses wird dann in
	//pWaitingCurrShell eingetragen und vom letzten DTor der CurrShell
	//"aktiviert".
	//Ein weiteres Problem ist das Zerstoeren einer Shell waehrend sie aktiv
	//ist. Der Pointer pCurrShell wird dann auf eine beliebige andere Shell
	//umgesetzt.
	//Wenn zum Zeitpunkt der Zerstoerung einer Shell diese noch in irgendwelchen
	//CurrShell-Objekten referenziert wird, so wird auch dies aufgeklart.
	friend class CurrShell;
	friend void SetShell( ViewShell *pSh );
	friend void InitCurrShells( SwRootFrm *pRoot );
	ViewShell *pCurrShell;
	ViewShell *pWaitingCurrShell;
	SwCurrShells *pCurrShells;

	//Eine Page im DrawModel pro Dokument, hat immer die Groesse der Root.
	SdrPage *pDrawPage;

	SwDestroyList* pDestroy;

	sal_uInt16	nPhyPageNums; // Number of pages
    sal_uInt16 nAccessibleShells; // Number of accessible shells

	void ImplCalcBrowseWidth();
	void ImplInvalidateBrowseWidth();

	void _DeleteEmptySct(); // zerstoert ggf. die angemeldeten SectionFrms
	void _RemoveFromList( SwSectionFrm* pSct ); // entfernt SectionFrms aus der Delete-Liste

protected:

	virtual void MakeAll();

public:

	//MasterObjekte aus der Page entfernen (von den Ctoren gerufen).
	static void RemoveMasterObjs( SdrPage *pPg );

	void AllCheckPageDescs() const;//swmod 080226
	void AllInvalidateAutoCompleteWords() const;//swmod 080305
	void AllAddPaintRect() const;
	void AllRemoveFtns() ;//swmod 080305
	void AllInvalidateSmartTagsOrSpelling(sal_Bool bSmartTags) const;//swmod 080307
	//Virtuelles Device ausgeben (z.B. wenn Animationen ins Spiel kommen)
	static sal_Bool FlushVout();
	//Clipping sparen, wenn im Vout eh genau das Cliprechteck ausgegeben wird
	static sal_Bool HasSameRect( const SwRect& rRect );

	SwRootFrm( SwFrmFmt*, ViewShell* );
	~SwRootFrm();
	void Init(SwFrmFmt*);

	ViewShell *GetCurrShell() const { return pCurrShell; }
	void DeRegisterShell( ViewShell *pSh );

	//Start-/EndAction fuer alle Shells auf moeglichst hoher
	//(Shell-Ableitungs-)Ebene aufsetzen. Fuer die StarONE Anbindung, die
	//die Shells nicht direkt kennt.
	//Der ChangeLinkd der CrsrShell (UI-Benachrichtigung) wird im EndAllAction
	//automatisch gecallt.
	void StartAllAction();
	void EndAllAction( sal_Bool bVirDev = sal_False );

	// fuer bestimmte UNO-Aktionen (Tabellencursor) ist es notwendig, dass alle Actions
	// kurzfristig zurueckgesetzt werden. Dazu muss sich jede ViewShell ihren alten Action-zaehler
	// merken
	void UnoRemoveAllActions();
	void UnoRestoreAllActions();

	const SdrPage* GetDrawPage() const { return pDrawPage; }
		  SdrPage* GetDrawPage()	   { return pDrawPage; }
		  void	   SetDrawPage( SdrPage* pNew ){ pDrawPage = pNew; }

	virtual sal_Bool  GetCrsrOfst( SwPosition *, Point&,
							   SwCrsrMoveState* = 0 ) const;

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
#ifdef DBG_UTIL
	virtual void Cut();
	virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

	virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    Point  GetNextPrevCntntPos( const Point &rPoint, sal_Bool bNext ) const;

	virtual Size ChgSize( const Size& aNewSize );

    void SetIdleFlags() { bIdleFormat = sal_True; }
    sal_Bool IsIdleFormat()  const { return bIdleFormat; }
    void ResetIdleFormat()     { bIdleFormat = sal_False; }

    bool IsNeedGrammarCheck() const         { return mbNeedGrammarCheck; }
    void SetNeedGrammarCheck( bool bVal )   { mbNeedGrammarCheck = bVal; }

	//Sorgt dafuer, dass alle gewuenschten Seitengebunden Flys eine Seite finden
	void SetAssertFlyPages() { bAssertFlyPages = sal_True; }
	void AssertFlyPages();
	sal_Bool IsAssertFlyPages()  { return bAssertFlyPages; }

	//Stellt sicher, dass ab der uebergebenen Seite auf allen Seiten die
	//Seitengebundenen Rahmen auf der richtigen Seite (Seitennummer) stehen.
	void AssertPageFlys( SwPageFrm * );

	//Saemtlichen Inhalt invalidieren, Size oder PrtArea
	void InvalidateAllCntnt( sal_uInt8 nInvalidate = INV_SIZE );

    /** method to invalidate/re-calculate the position of all floating
        screen objects (Writer fly frames and drawing objects), which are
        anchored to paragraph or to character.

        OD 2004-03-16 #i11860#

        @author OD
    */
    void InvalidateAllObjPos();

	//Ueberfluessige Seiten entfernen.
	void SetSuperfluous()	   { bCheckSuperfluous = sal_True; }
	sal_Bool IsSuperfluous() const { return bCheckSuperfluous; }
	void RemoveSuperfluous();

	//abfragen/setzen der aktuellen Seite und der Gesamtzahl der Seiten.
	//Es wird soweit wie notwendig Formatiert.
	sal_uInt16	GetCurrPage( const SwPaM* ) const;
	sal_uInt16	SetCurrPage( SwCursor*, sal_uInt16 nPageNum );
	Point	GetPagePos( sal_uInt16 nPageNum ) const;
	sal_uInt16	GetPageNum() const 		{ return nPhyPageNums; }
	void	DecrPhyPageNums()		{ --nPhyPageNums; }
	void	IncrPhyPageNums()		{ ++nPhyPageNums; }
	sal_Bool	IsVirtPageNum() const	{ return bIsVirtPageNum; }
	inline	void SetVirtPageNum( const sal_Bool bOf ) const;
    sal_Bool    IsDummyPage( sal_uInt16 nPageNum ) const;

    // Point rPt: The point that should be used to find the page
    // Size pSize: If given, we return the (first) page that overlaps with the
    // rectangle defined by rPt and pSize
    // bool bExtend: Extend each page to the left/right/top/bottom up to the
    // next page border
    const SwPageFrm* GetPageAtPos( const Point& rPt, const Size* pSize = 0, bool bExtend = false ) const;

    void CalcFrmRects( SwShellCrsr& );

    // Calculates the cells included from the current selection
    // false: There was no result because of an invalid layout
    // true: Everything worked fine.
    bool MakeTblCrsrs( SwTableCursor& );

	void DisallowTurbo()  const { ((SwRootFrm*)this)->bTurboAllowed = sal_False; }
	void ResetTurboFlag() const { ((SwRootFrm*)this)->bTurboAllowed = sal_True; }
	sal_Bool IsTurboAllowed() const { return bTurboAllowed; }
	void SetTurbo( const SwCntntFrm *pCntnt ) { pTurbo = pCntnt; }
	void ResetTurbo() { pTurbo = 0; }
	const SwCntntFrm *GetTurbo() { return pTurbo; }

	//Fussnotennummern aller Seiten auf den neuesten Stand bringen.
	void UpdateFtnNums();			//nur bei Seitenweiser Nummerierung!

	//Alle Fussnoten (nicht etwa die Referenzen) entfernen.
	void RemoveFtns( SwPageFrm *pPage = 0, sal_Bool bPageOnly = sal_False,
					 sal_Bool bEndNotes = sal_False );
	void CheckFtnPageDescs( sal_Bool bEndNote );

	const SwPageFrm *GetLastPage() const { return pLastPage; }
		  SwPageFrm *GetLastPage() 		 { return pLastPage; }

	static sal_Bool IsInPaint() { return bInPaint; }

	static void SetNoVirDev( const sal_Bool bNew ) { bNoVirDev = bNew; }

	inline long GetBrowseWidth() const;
	void SetBrowseWidth( long n ) { bBrowseWidthValid = sal_True; nBrowseWidth = n;}
	inline void InvalidateBrowseWidth();

#ifdef LONG_TABLE_HACK
	void HackPrepareLongTblPaint( int nMode );
#endif

	sal_Bool IsNewLayout() const { return bIsNewLayout; }
	void ResetNewLayout() 	 { bIsNewLayout = sal_False;}

	// Hier werden leere SwSectionFrms zur Zerstoerung angemeldet
	// und spaeter zerstoert oder wieder abgemeldet
	void InsertEmptySct( SwSectionFrm* pDel );
	void DeleteEmptySct() { if( pDestroy ) _DeleteEmptySct(); }
	void RemoveFromList( SwSectionFrm* pSct ) { if( pDestroy ) _RemoveFromList( pSct ); }
#ifdef DBG_UTIL
	// Wird zur Zeit nur fuer ASSERTs benutzt:
	sal_Bool IsInDelList( SwSectionFrm* pSct ) const; // Ist der SectionFrm in der Liste enthalten?
#endif


	void SetCallbackActionEnabled( sal_Bool b ) { bCallbackActionEnabled = b; }
	sal_Bool IsCallbackActionEnabled() const	{ return bCallbackActionEnabled; }

	sal_Bool IsAnyShellAccessible() const { return nAccessibleShells > 0; }
	void AddAccessibleShell() { ++nAccessibleShells; }
	void RemoveAccessibleShell() { --nAccessibleShells; }

    /** get page frame by physical page number

        OD 14.01.2003 #103492#
        looping through the lowers, which are page frame, in order to find the
        page frame with the given physical page number.
        if no page frame is found, 0 is returned.
        Note: Empty page frames are also returned.

        @param _nPageNum
        input parameter - physical page number of page frame to be searched and
        returned.

        @return pointer to the page frame with the given physical page number
    */
    SwPageFrm* GetPageByPageNum( sal_uInt16 _nPageNum ) const;

    // --> PAGES01
    void CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea );
    bool IsLeftToRightViewLayout() const;
    const SwRect& GetPagesArea() const { return maPagesArea; }
    void SetSidebarChanged() { mbSidebarChanged = true; }
    // <--
};

inline long SwRootFrm::GetBrowseWidth() const
{
	if ( !bBrowseWidthValid )
		((SwRootFrm*)this)->ImplCalcBrowseWidth();
	return nBrowseWidth;
}

inline void SwRootFrm::InvalidateBrowseWidth()
{
	if ( bBrowseWidthValid )
		ImplInvalidateBrowseWidth();
}

inline	void SwRootFrm::SetVirtPageNum( const sal_Bool bOf) const
{
	((SwRootFrm*)this)->bIsVirtPageNum = bOf;
}

#endif  // SW_ROOTFRM_HXX

/* vim: set noet sw=4 ts=4: */
