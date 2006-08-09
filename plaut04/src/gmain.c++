#include "gplaut04.h"
#include "gVarNames.h"
#include "tube.h"

#define NUM_SP_POINTS 13
#define SP_LBL_ITEMS 4
#define CL_SP_ITEMS  6
#define LBL_OFFSET   4

#define TIME_IS_OFF  0 
#define TIME_ON_X    1
#define TIME_ON_Y    2
#define TIME_ON_Z    3

float STATIONARY_POINT_RADIUS = 0.01;
int specialColorItems = CL_SP_ITEMS;

extern void  rounding(double &, double &);

struct ViewerAndScene
{
    SoXtExaminerViewer *viewer;
    char               *filename;
    SoNode             *scene;
} ;

ViewerAndScene *vwrAndScene;

float fmData[12];
char autoDir[256];

GC gc;
Colormap colormap;
XColor black, grey, red, white, green, blue, exact;

static int iiii = 0;

SbColor lineColor[NUM_SP_POINTS];
SbColor lineColorTemp[NUM_SP_POINTS];
SbColor lineColorOld[NUM_SP_POINTS];
SbColor envColors[10];

int bifDefaultAxis[3], solDefaultAxis[3];

struct DefaultAxisItems
{
    int solXSize, solYSize, solZSize;
    int bifXSize, bifYSize, bifZSize;
    int bifX[MAX_PAR], bifY[MAX_PAR], bifZ[MAX_PAR];
    int solX[MAX_PAR], solY[MAX_PAR], solZ[MAX_PAR];
} dai;


unsigned long linePattern[NUM_SP_POINTS];
unsigned long linePatternTemp[NUM_SP_POINTS];
unsigned long linePatternOld[NUM_SP_POINTS];

unsigned long stabilityLinePattern[2];
unsigned long stabilityLinePatternTemp[2];

char *systemLinePatternLookAndFeel[] =
{
    "SOLID LINE",   "--------",   ". . . . . ",    "_ . _ . _ .",
    "_ . . _ . .",  "_ . . . _",  "___ _ ___ _", "____ __ ____",
    "NULL "
};

unsigned long systemLinePatternValue[] =
{
    0xfffff, 0x7777,  0x3333,  0xfafa, 0xeaea, 0xffcc, 0xffdc,0xff9c,0
};

bool blOpenSolFile = TRUE;
bool blOpenBifFile = TRUE;

bool blDrawTicker = false;

int whichType= 0 ;
int whichTypeTemp= 0 ;
int whichTypeOld = 0 ;

int whichStyle= 0 ;
int whichStyleTemp= 0 ;
int whichStyleOld = 0 ;

int whichCoord = 0 ;
int whichCoordTemp = 0 ;
int whichCoordOld = 0 ;
int time_on = 0;

unsigned long graphWidgetToggleSet     = (unsigned long) 0 ;
unsigned long graphWidgetToggleSetTemp = (unsigned long) 0 ;
unsigned long graphWidgetToggleSetOld  = (unsigned long) 0 ;

SolNode mySolNode;
BifNode myBifNode;
UserData clientData;

bool options[]=
{
    TRUE,   TRUE, FALSE, TRUE,  FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE
};
bool optionsTemp[11], optionsOld[11], optBif[11], optSol[11];
bool setShow3D, setShow3DSol, setShow3DBif;

char sFileName[256];
char bFileName[256];
char dFileName[256];

Widget  topform;
Widget  xAxisList, yAxisList, zAxisList, labelsList, colorMethodSeletionList;
Widget satAniSpeedSlider, orbitAniSpeedSlider;

solutionp solHead = NULL;
long int animationLabel = 0;
int maxComponent = 1;
int curComponent = 1;

int winWidth, winHeight;

int xCoordIndices[MAX_LIST], xCoordIdxSize;
int yCoordIndices[MAX_LIST], yCoordIdxSize;
int zCoordIndices[MAX_LIST], zCoordIdxSize;
int lblIndices[MAX_LABEL], lblChoice[MAX_LABEL], lblIdxSize;

int MAX_SAT_SPEED = 1000;
int MIN_SAT_SPEED = 0;
int MAX_ORBIT_SPEED = 1000;
int MIN_ORBIT_SPEED = 0;

float orbitSpeed = 1.0;
float satSpeed   = 0.5;
float satRadius  = 1.0;
float lineWidthScaler = 1.0;
float aniLineScaler = 2.0;
int   coloringMethod = -1;
float bgTransparency = 0;
float numPeriodAnimated = 1.0;

long int numLabels;

double legendScaleValues[2];

SoSeparator *userScene;
SoSeparator *root;
SoSeparator *starryBackground;

typedef struct EditMenuItems
{
    Widget *items;
    int     which;
} EditMenuItems;

EditMenuItems *typeMenuItems, *styleMenuItems, *coordMenuItems;

Widget fileDialog = NULL;

static char xAxis[MAX_LIST][5];
static char zAxis[MAX_LIST][5];
static char yAxis[MAX_LIST][5];
static char labels[MAX_LABEL][8];

static char coloringMethodList[MAX_LIST+CL_SP_ITEMS][8];
int myLabels[MAX_LABEL+SP_LBL_ITEMS];

XmStringTable xList = (XmStringTable) 0 ;
XmStringTable yList = (XmStringTable) 0 ;
XmStringTable zList = (XmStringTable) 0 ;
XmStringTable lblList = (XmStringTable) 0 ;
XmStringTable clrMethodList= (XmStringTable) 0 ;

///////////////////////////////////////////////////////////////
//
//  Function prototypes
//
void createPreferDialog();
char *copyenv(char *name);

SoSeparator * drawAStrip(float stripSet[][3], int size);
SoSeparator * drawATube(TubeNode cnode);
SoSeparator * drawASphere(float ptb[], float size);
static void xListCallBack(Widget combo, XtPointer client_data, XtPointer call_data);
static void yListCallBack(Widget combo, XtPointer client_data, XtPointer call_data);
static void zListCallBack(Widget combo, XtPointer client_data, XtPointer call_data);
static void lblListCallBack(Widget combo, XtPointer client_data, XtPointer call_data);
void fileDialogCB(Widget, XtPointer client_data, XtPointer data);
SbBool readFile(char *filename);
SoSeparator * MyFileRead(const char *filename, SbString &errorMessage);
void deleteScene();
void writeToFile();
void getFileName(int fileMode);
void setListValue();
void showHelpDialog();
void showAboutDialog();
void myMousePressCB(void *userData, SoEventCallback *eventCB);
void hidenDialogShell (Widget widget, XtPointer client_data, XtPointer call_data);
void applyPreferDialogChangeAndUpdateScene(Widget widget, XtPointer client_data, XtPointer call_data);
void closePreferDialogAndUpdateScene(Widget widget, XtPointer client_data, XtPointer call_data);
void closePreferDialogAndGiveUpChange(Widget widget, XtPointer client_data, XtPointer call_data);
void savePreferAndUpdateScene(Widget widget, XtPointer client_data, XtPointer call_data);
void setLegendValues(double* values);

void lookForThePoint(float position[],long int &bIdx, long int &sIdx);

SoSeparator * createStarryBackground(int total,float diameter);
void updateScene();
SoGroup * setLineAttributesByStability(int stability, float scaler);
SoGroup * setLineColorBlendingByStability(float * vertices, long int size, int stab, float scaler);
SoGroup * setLineAttributesByParameterValue(double parValue, double parMax, double parMid, double parMin, int stability, float scaler);
SoGroup * setLineAttributesByBranch(int iBranch, int stability, float scaler);
SoGroup * setLineAttributesByType(int stability, int type, float scaler);
SoGroup * setLineColorBlending(float * vertices, long int size, int stability, int type, float scaler);

SoSeparator * createCoordinates(bool, int type, float mx[3], float mn[3], int tk[3], int where);
SoSeparator * createLegend(SbVec3f pos, double val[5]);
SoSeparator * createDiscreteLegend(SbVec3f pos, SbColor lineColors[13]);
SoSeparator * createBranchLegend(SbVec3f pos, SbColor lineColors[13]);
SoSeparator * createStabilityLegend(SbVec3f pos, SbColor lineColors[2]);
SoSeparator * addLegend();

SoSeparator * createSolutionSceneWithWidgets();
SoSeparator * createBifurcationScene();
SoSeparator * renderSolution();
SoSeparator * renderBifurcation();
SoSeparator * animateSolutionUsingTubes(bool aniColoring);
SoSeparator * animateSolutionUsingLines(bool aniColoring);
SoSeparator * animateSolutionUsingPoints(int style, bool aniColoring);
SoSeparator * animateOrbitMovement(long int n, long int si);
SoSeparator * animateOrbitWithTail(int iBranch, long int j,long  int si);
SoSeparator * drawAnOrbitUsingLines(int iBranch, long int l, long int si, float scaler, int stability, int type, bool coloring);
SoSeparator * drawAnOrbitUsingPoints(int style, int iBranch,  long int l, long int si, float scaler, int stability, int type, bool aniColoring);
SoSeparator * drawAnOrbitUsingNurbsCurve(int iBranch, long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawAnOrbitUsingTubes(int iBranch, long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawABifBranchUsingLines(int iBranch, long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawABifBranchUsingNurbsCurve(int iBranch,long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawABifLabelInterval(long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawABifLabelIntervalUsingNurbsCurve(long int l, long int si, float scaler, int stability, int type);
SoSeparator * drawASphereWithColor(float color[], float position[], float size);
SoSeparator * drawASolBranchUsingSurface(long obStart, long obEnd, long numVert);

SoSeparator * createAxis(float red, float green, float blue);
SoMaterial  * setLabelMaterial(int lblType);
SoSeparator * drawStarryBackground(char * bgFileName);

int readResourceParameters();
int writePreferValuesToFile();
//void initLabelList();
void copySolDataToWorkArray(int varIndices[]);
void searchForMaxMin(int component, int  varIndices[]);
void copyBifDataToWorkArray(int varIndices[]);

static void processPrinting(char* filename );
void postDeals();

extern SoSeparator * createBoundingBox();
extern char * strrighttrim(char*);
extern char * strlefttrim(char*);

//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
//  functions
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
void
orbitSpeedCB(Widget, XtPointer userData, XtPointer callData)
//
////////////////////////////////////////////////////////////////////////
{
    XmScaleCallbackStruct *data = (XmScaleCallbackStruct *) callData;
    orbitSpeed = data->value/50.0;                ///50.0;     ///75.0;
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
satSpeedCB(Widget, XtPointer userData, XtPointer callData)
//
////////////////////////////////////////////////////////////////////////
{
    XmScaleCallbackStruct *data = (XmScaleCallbackStruct *) callData;
    satSpeed = data->value/100.0;
    updateScene();
}

////////////////////////////////////////////////////////////////////////
//
void
numPeriodAnimatedCB(Widget, XtPointer userData, XtPointer callData)
//
////////////////////////////////////////////////////////////////////////
{
    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)callData;
    char *myChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);
    int i = 0;

    if ( strcmp(myChoice, "inf") == 0 )
    {
       numPeriodAnimated = -1; 
    } 
    else 
    {
       numPeriodAnimated = atof(myChoice);
    }

//cout <<" Num Period Animated "<<myChoice<<"   "<<numPeriodAnimated;

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
colorMethodSelectionCB(Widget, XtPointer userData, XtPointer callData)
//
////////////////////////////////////////////////////////////////////////
{
    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)callData;
    char *myChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);
    int i = 0;
    int choice = (int) cbs->item_position;

    coloringMethod = (strcasecmp(myChoice,"COMP")==0) ?  CL_COMPONENT:
    ((strcasecmp(myChoice,"TYPE")==0) ?  CL_ORBIT_TYPE :
    ((strcasecmp(myChoice,"BRAN")==0) ? CL_BRANCH_NUMBER:
    ((strcasecmp(myChoice,"PONT")==0) ? CL_POINT_NUMBER :
    ((strcasecmp(myChoice,"LABL")==0) ? CL_LABELS:
    ((strcasecmp(myChoice,"STAB")==0) ? CL_STABILITY : choice - specialColorItems)))));

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
lineWidthCB(Widget, XtPointer userData, XtPointer callData)
//
////////////////////////////////////////////////////////////////////////
{
    Widget        spin = (Widget) userData;
    unsigned char type;
    int           position;

    XtVaGetValues (spin, XmNspinBoxChildType, &type, XmNposition, &position, NULL);
    lineWidthScaler = position/10.0;
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void 
cropScene(char* filename)
//
////////////////////////////////////////////////////////////////////////
{
    char command[100];
    system("import my_temp.gif");
    system("convert -crop 1024x768 my_temp.gif my_temp_cropped.gif");
    system("convert my_temp_cropped.gif my_temp.eps");
    system("rm my_temp.gif my_temp_cropped.gif");
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt when a menu item is picked from the File menu.
//
static void
fileMenuPick(Widget, void *userData, XtPointer *)
//
////////////////////////////////////////////////////////////////////////
{
    int    which = (long) userData;

    switch (which)
    {
        case SAVE_ITEM:
            getFileName(SAVE_ITEM);
            break;

        case QUIT_ITEM:
            postDeals();
            exit(0);
            break;
        case PRINT_ITEM:
            cropScene("myfile");
            break;
        case OPEN_ITEM:
            getFileName(OPEN_ITEM);
            break;
        default:
            printf("UNKNOWN file menu item!!!\n"); break;
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt when a menu item is picked from the TYPE menu.
//
static void
typeMenuPick(Widget w, void *userData, XmAnyCallbackStruct *cb)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (long) userData;

// get the user data for this menu item.
    Arg     args[1];
    EditMenuItems *menuItems;
    XtSetArg(args[0], XmNuserData, &menuItems);
    XtGetValues(w, args, 1);

    menuItems->which = which;
    whichType    = which;

    if ( whichType != whichTypeOld )
    {
        if( whichType != BIFURCATION )
        {
            setShow3DBif = setShow3D;
            setShow3D = setShow3DSol;
            for(int i=0; i<11; ++i)
            {
                optBif[i]  = options[i];
                options[i] = optSol[i];
            }
        }
        else
        {
            setShow3DSol = setShow3D;
            setShow3D = setShow3DBif;
            for(int i=0; i<11; ++i)
            {
                optSol[i]  = options[i];
                options[i] = optBif[i];
            }
        }
    }

    whichTypeOld = whichType;

    setListValue();
    XtVaSetValues(xAxisList, XmNselectedPosition, xCoordIndices[0], NULL);
    XtVaSetValues(yAxisList, XmNselectedPosition, yCoordIndices[0], NULL);
    XtVaSetValues(zAxisList, XmNselectedPosition, zCoordIndices[0], NULL);

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//       This is called by Xt when a menu item is picked from the Option menu.
//
static void
optMenuPick(Widget widget, void *userData, XmAnyCallbackStruct *cb)  
//
////////////////////////////////////////////////////////////////////////
{
    XmToggleButtonCallbackStruct *toggle = (XmToggleButtonCallbackStruct *)cb;
    int which = (long) userData;

    if (toggle->set == XmSET)
    {
        graphWidgetToggleSet |= (1 << which);
        options[which] = true;
    }
    else
    {
        options[which] = false;
        graphWidgetToggleSet &= ~(1 << which);
    }

    if(options[OPT_SAT_ANI])
        XtVaSetValues (satAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (satAniSpeedSlider, XmNeditable, FALSE, NULL);

    if(options[OPT_PERIOD_ANI])
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, FALSE, NULL);

    updateScene();
}


///////////////////////////////////////////////////////////////////////////
//
void
setListValue()
//
///////////////////////////////////////////////////////////////////////////
{
    long nItems = (whichType != BIFURCATION) ? 
	                   mySolNode.totalLabels+SP_LBL_ITEMS : 
					   myBifNode.totalLabels+SP_LBL_ITEMS;
    int count = XtNumber (labels);

    lblList = (XmStringTable) XtMalloc(count * sizeof (XmString *));
    for (int i = 0; i < count; i++)
        lblList[i] = XmStringCreateLocalized (labels[i]);

    if(whichType != BIFURCATION)
    {
        xCoordIdxSize = dai.solXSize;
        yCoordIdxSize = dai.solYSize;
        zCoordIdxSize = dai.solZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.solX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.solY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.solZ[i];

        XtVaSetValues(xAxisList, XmNitems, xList, XmNitemCount, mySolNode.nar, NULL);
        XtVaSetValues(yAxisList, XmNitems, yList, XmNitemCount, mySolNode.nar, NULL);
        XtVaSetValues(zAxisList, XmNitems, zList, XmNitemCount, mySolNode.nar, NULL);

        int sp = 0;
        strcpy(coloringMethodList[0],"STAB"); sp++;
        strcpy(coloringMethodList[1],"PONT"); sp++;
        strcpy(coloringMethodList[2],"BRAN"); sp++;
        strcpy(coloringMethodList[3],"TYPE"); sp++;
        strcpy(coloringMethodList[4],"LABL"); sp++;
        strcpy(coloringMethodList[5],"COMP"); sp++;
        specialColorItems = sp;
        for(int i=sp; i<mySolNode.nar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"%d",i-sp);
        }
        for(int i=mySolNode.nar+sp; i<mySolNode.nar+mySolNode.npar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"par[%d]",mySolNode.parID[i-(mySolNode.nar+sp)]);
        }

        int count = XtNumber (coloringMethodList);
        for (int i = 0; i < count; ++i)
            clrMethodList[i] = XmStringCreateLocalized (coloringMethodList[i]);

        XtVaSetValues(colorMethodSeletionList, XmNitems, clrMethodList, 
		               XmNitemCount, mySolNode.nar+mySolNode.npar+sp, NULL);
        XtVaSetValues(labelsList, XmNitems, lblList, XmNitemCount, nItems, NULL);
    }
    else
    {
        xCoordIdxSize = dai.bifXSize;
        yCoordIdxSize = dai.bifYSize;
        zCoordIdxSize = dai.bifZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.bifX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.bifY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.bifZ[i];
        XtVaSetValues(xAxisList, XmNitems, xList, XmNitemCount, myBifNode.nar, NULL);
        XtVaSetValues(yAxisList, XmNitems, yList, XmNitemCount, myBifNode.nar, NULL);
        XtVaSetValues(zAxisList, XmNitems, zList, XmNitemCount, myBifNode.nar, NULL);

        int sp = 0;
        strcpy(coloringMethodList[0],"STAB"); sp++;
        strcpy(coloringMethodList[1],"PONT"); sp++;
        strcpy(coloringMethodList[2],"BRAN"); sp++;
        specialColorItems = sp;
        for(int i=sp; i<myBifNode.nar+sp; ++i)
        {
            sprintf(coloringMethodList[i],"%d",i-sp);
        }
        int count = XtNumber (coloringMethodList);
        for (int i = 0; i < count; ++i)
            clrMethodList[i] = XmStringCreateLocalized (coloringMethodList[i]);

        XtVaSetValues(colorMethodSeletionList, XmNitems, clrMethodList, XmNitemCount, myBifNode.nar+sp,NULL);
        XtVaSetValues(labelsList, XmNitems, lblList, XmNitemCount, nItems, NULL);
    }

    if(setShow3D)
        XtSetSensitive (zAxisList, true);
    else
    {
        for(int i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;
        zCoordIdxSize = 1;
        XtSetSensitive (zAxisList, false);
    }

}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt when a menu item is picked from the STYLE menu.
//
static void
styleMenuPick(Widget w, void *userData, XmAnyCallbackStruct *cb)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (long) userData;

// get the user data for this menu item.
    Arg     args[1];
    EditMenuItems *menuItems;
    XtSetArg(args[0], XmNuserData, &menuItems);
    XtGetValues(w, args, 1);

    menuItems->which = which;
    whichStyle = which;
    whichStyleOld = whichStyle;

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt when a menu item is picked from the STYLE menu.
//
static void
coordMenuPick(Widget w, void *userData, XmAnyCallbackStruct *cb)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (long) userData;

// get the user data for this menu item.
    Arg     args[1];
    EditMenuItems *menuItems;
    XtSetArg(args[0], XmNuserData, &menuItems);
    XtGetValues(w, args, 1);

    menuItems->which = which;

    if(which == DRAW_TICKER)
        blDrawTicker = !blDrawTicker ;
    else
    {
        whichCoord = which;
        whichCoordOld = whichCoord;
    }

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt just before the TYPE menu is displayed.
//
static void 
typeMenuDisplay(Widget, void *userData, XtPointer)
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = (EditMenuItems *) userData;

    TOGGLE_OFF(menuItems->items[SOLUTION]);
    TOGGLE_OFF(menuItems->items[BIFURCATION]);

    TOGGLE_ON(menuItems->items[menuItems->which]);

    XtSetSensitive (menuItems->items[SOLUTION], TRUE);
    XtSetSensitive (menuItems->items[BIFURCATION], TRUE);

    if(!blOpenSolFile)
        XtSetSensitive (menuItems->items[SOLUTION], FALSE);

    if(!blOpenBifFile)
        XtSetSensitive (menuItems->items[BIFURCATION], FALSE);

}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt just before the STYLE menu is displayed.
//
static void 
styleMenuDisplay(Widget, void *userData, XtPointer)
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = (EditMenuItems *) userData;
    TOGGLE_OFF(menuItems->items[LINE]);
    TOGGLE_OFF(menuItems->items[TUBE]);
    TOGGLE_OFF(menuItems->items[SURFACE]);
    TOGGLE_OFF(menuItems->items[MESH_POINTS]);
    TOGGLE_OFF(menuItems->items[ALL_POINTS]);

    if( whichType == BIFURCATION)
    {
        XtSetSensitive (menuItems->items[SURFACE], false);
        XtSetSensitive (menuItems->items[MESH_POINTS], false);
        XtSetSensitive (menuItems->items[ALL_POINTS], false);
    }
    else
    {
        XtSetSensitive (menuItems->items[SURFACE], true);
        XtSetSensitive (menuItems->items[MESH_POINTS], true);
        XtSetSensitive (menuItems->items[ALL_POINTS], true);
    }

    TOGGLE_ON(menuItems->items[menuItems->which]);

}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt just before the STYLE menu is displayed.
//
static void 
coordMenuDisplay(Widget, void *userData, XtPointer)
//
////////////////////////////////////////////////////////////////////////
{
    EditMenuItems *menuItems = (EditMenuItems *) userData;

    if(menuItems->which == DRAW_TICKER)
    {
        if(blDrawTicker)
            XmToggleButtonSetState(menuItems->items[menuItems->which], TRUE, FALSE);
        else
            XmToggleButtonSetState(menuItems->items[menuItems->which], FALSE, FALSE);
    }
    else
    {
        TOGGLE_OFF(menuItems->items[NO_COORD]);
        TOGGLE_OFF(menuItems->items[COORDORIGIN]);
        TOGGLE_OFF(menuItems->items[LEFTBACK]);
        TOGGLE_OFF(menuItems->items[LEFTAHEAD]);

        TOGGLE_ON(menuItems->items[menuItems->which]);
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This is called by Xt just before the TYPE menu is displayed.
//
static void 
optMenuDisplay(Widget, void *userData, XtPointer)
//
////////////////////////////////////////////////////////////////////////
{
    Arg args[3];
    int n = 0;
    EditMenuItems *menuItems = (EditMenuItems *) userData;

    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        n = 0;
        if (graphWidgetToggleSet & (1<<i))
        {
            XmToggleButtonSetState(menuItems->items[i], TRUE, FALSE);
        }
        else
        {
            XmToggleButtonSetState(menuItems->items[i], FALSE, FALSE);
        }
    }

    XmString xString = XmStringCreateLocalized("draw Label");

    if(whichType == SOLUTION)
    {
        XtSetSensitive (menuItems->items[OPT_SAT_ANI], TRUE);

        XmString xString = XmStringCreateLocalized("Highlight Orbit");
        XtVaSetValues (menuItems->items[OPT_PERIOD_ANI], XmNlabelString, xString, NULL);
        XmStringFree(xString);
    }
    else
    {
        XtSetSensitive (menuItems->items[OPT_SAT_ANI], FALSE);

        XmString xString = XmStringCreateLocalized("Draw Labels");
        XtVaSetValues (menuItems->items[OPT_PERIOD_ANI], XmNlabelString, xString, NULL);
        XmStringFree(xString);
    }
}


// A push item has no on/off state - it merely gets selected
#define PUSH_ITEM(ITEM,NAME,KONST,FUNC) \
    ITEM = XtCreateManagedWidget(NAME, \
    xmPushButtonGadgetClass, pulldown, args, n); \
    XtAddCallback(ITEM, XmNactivateCallback,\
    (XtCallbackProc) FUNC, \
    (XtPointer) KONST)

// A toggle item has on/off state
#define TOGGLE_ITEM(ITEM,NAME,KONST,FUNC) \
    ITEM = XtCreateManagedWidget(NAME, \
    xmToggleButtonGadgetClass, pulldown, args, n); \
    XtAddCallback(ITEM, XmNvalueChangedCallback,\
    (XtCallbackProc) FUNC, \
    (XtPointer) KONST)

// A separator draws a line between menu items
#define SEP_ITEM(NAME) \
    (void) XtCreateManagedWidget("separator", \
    xmSeparatorGadgetClass, pulldown, NULL, 0)

////////////////////////////////////////////////////////////////////////
//
//  This creates the File menu and all its items.
//
Widget
buildFileMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  items[4];
    Widget  pulldown;
    Arg     args[8];
    int        n;

    Arg popupargs[2];
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "fileMenu", popupargs, popupn);

/*
    char *openAccel  = "Alt <Key> o";
    char *saveAccel  = "Alt <Key> s";
    char *printAccel = "Alt <Key> p";
    char *quitAccel  = "Alt <Key> q";
    XmString openAccelText  = XmStringCreate("Alt+o", XmSTRING_DEFAULT_CHARSET);
    XmString saveAccelText  = XmStringCreate("Alt+s", XmSTRING_DEFAULT_CHARSET);
    XmString printAccelText = XmStringCreate("Alt+p", XmSTRING_DEFAULT_CHARSET);
    XmString quitAccelText  = XmStringCreate("Alt+q", XmSTRING_DEFAULT_CHARSET);
*/
    n = 0;
//    XtSetArg(args[n], XmNaccelerator, openAccel); n++;
//    XtSetArg(args[n], XmNacceleratorText, openAccelText); n++;
    PUSH_ITEM(items[0], "Open...", OPEN_ITEM, fileMenuPick);

    n = 0;
//    XtSetArg(args[n], XmNaccelerator, saveAccel); n++;
//    XtSetArg(args[n], XmNacceleratorText, saveAccelText); n++;
    PUSH_ITEM(items[1], "Export...", SAVE_ITEM, fileMenuPick);

    n = 0;
//    XtSetArg(args[n], XmNaccelerator, printAccel); n++;
//    XtSetArg(args[n], XmNacceleratorText, printAccelText); n++;
//    PUSH_ITEM(items[2], "Print...", PRINT_ITEM, fileMenuPick);

    SEP_ITEM("separator");

    n = 0;
//    XtSetArg(args[n], XmNaccelerator, quitAccel); n++;
//    XtSetArg(args[n], XmNacceleratorText, quitAccelText); n++;
//    PUSH_ITEM(items[3], "Quit",    QUIT_ITEM, fileMenuPick);
    PUSH_ITEM(items[2], "Quit",    QUIT_ITEM, fileMenuPick);

//    XtManageChildren(items, 4);
    XtManageChildren(items, 2);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the Help menu and all its items.
//
Widget
buildHelpMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  items[2];
    Widget  pulldown;
    Arg     args[8];
    int     n;

    Arg popupargs[2];
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "helpMenu", popupargs, popupn);

    n = 0;
    PUSH_ITEM(items[0], "About", ITEM_ONE, showAboutDialog);

    SEP_ITEM("separator");

    n = 0;
    PUSH_ITEM(items[1], "HELP", ITEM_TWO, showHelpDialog);
    XtManageChildren(items, 2);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the TYPE menu and all its items.
//
Widget
buildOptionMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  pulldown;
    Arg     args[13];
    int        n;

    EditMenuItems *menuItems = new EditMenuItems;
    menuItems->items = new Widget[6];

    Arg popupargs[13];
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "optionMenu", popupargs, popupn);

    XtAddCallback(pulldown, XmNmapCallback,
        (XtCallbackProc) optMenuDisplay, (XtPointer) menuItems);

    n = 0;
    int mq = 0;
    XtSetArg(args[n], XmNuserData, menuItems); n++;
    TOGGLE_ITEM(menuItems->items[mq], "Hightlight Orbit",     OPT_PERIOD_ANI, optMenuPick); ++mq;
    TOGGLE_ITEM(menuItems->items[mq], "Orbit Animation",      OPT_SAT_ANI,    optMenuPick); ++mq;
    TOGGLE_ITEM(menuItems->items[mq], "Draw Background",      OPT_BACKGROUND, optMenuPick); ++mq;
    TOGGLE_ITEM(menuItems->items[mq], "Add Legend",           OPT_LEGEND, optMenuPick); ++mq;
    TOGGLE_ITEM(menuItems->items[mq], "Normalize Data",       OPT_NORMALIZE_DATA, optMenuPick); ++mq;
    XtManageChildren(menuItems->items, mq);

    Widget pushitem;
    SEP_ITEM("separator");
    PUSH_ITEM(pushitem, "PREFERENCES", ITEM_ONE, createPreferDialog);
    XtManageChild(pushitem);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the STYLE menu and all its items.
//
Widget
buildStyleMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  pulldown;
    Arg     args[5];
    int        n;

    styleMenuItems = new EditMenuItems;
    styleMenuItems->items = new Widget[5];   
    styleMenuItems->which = whichStyle;

    Arg popupargs[5];                         
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "styleMenu", popupargs, popupn);

    XtAddCallback(pulldown, XmNmapCallback,
        (XtCallbackProc) styleMenuDisplay, (XtPointer) styleMenuItems);

    n = 0;
    XtSetArg(args[n], XmNuserData, styleMenuItems); n++;
    XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); n++;
    TOGGLE_ITEM(styleMenuItems->items[0], "Line",    LINE, styleMenuPick);
    TOGGLE_ITEM(styleMenuItems->items[1], "Tube",    TUBE, styleMenuPick);
    TOGGLE_ITEM(styleMenuItems->items[2], "Surface", SURFACE, styleMenuPick);
    TOGGLE_ITEM(styleMenuItems->items[3], "Mesh Points", MESH_POINTS, styleMenuPick);
    TOGGLE_ITEM(styleMenuItems->items[4], "All Points", ALL_POINTS, styleMenuPick);

    XtManageChildren(styleMenuItems->items, 5);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the Coordinates menu and all its items.
//
Widget
buildCoordMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  pulldown;
    Arg     args[3];
    int        n;

    coordMenuItems = new EditMenuItems;
    coordMenuItems->items = new Widget[5];
    coordMenuItems->which = whichCoord;

    Arg popupargs[3];
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "coordMenu", popupargs, popupn);

    XtAddCallback(pulldown, XmNmapCallback,
        (XtCallbackProc) coordMenuDisplay, (XtPointer) coordMenuItems);

    n = 0;
    XtSetArg(args[n], XmNuserData, coordMenuItems); n++;
    XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); n++;
    TOGGLE_ITEM(coordMenuItems->items[0], "NONE",   NO_COORD, coordMenuPick);
    TOGGLE_ITEM(coordMenuItems->items[1], "Coord Center",    COORDORIGIN, coordMenuPick);
    TOGGLE_ITEM(coordMenuItems->items[2], "Left and Back ",   LEFTBACK, coordMenuPick);
    TOGGLE_ITEM(coordMenuItems->items[3], "Left and Ahead",   LEFTAHEAD, coordMenuPick);
    SEP_ITEM("separator");
    TOGGLE_ITEM(coordMenuItems->items[4], "Draw Scale",   DRAW_TICKER, coordMenuPick);

    XtManageChildren(coordMenuItems->items, 5);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the TYPE menu and all its items.
//
Widget
buildTypeMenu(Widget menubar)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  pulldown;
    Arg     args[2];
    int        n;

    typeMenuItems = new EditMenuItems;
    typeMenuItems->items = new Widget[2];
    typeMenuItems->which = whichType;

    Arg popupargs[2];
    int popupn = 0;

    pulldown = XmCreatePulldownMenu(menubar, "typeMenu", popupargs, popupn);

    XtAddCallback(pulldown, XmNmapCallback,
        (XtCallbackProc) typeMenuDisplay, (XtPointer) typeMenuItems);

    n = 0;
    XtSetArg(args[n], XmNuserData, typeMenuItems); n++;
    XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); n++;
    TOGGLE_ITEM(typeMenuItems->items[0], "Solution",    SOLUTION, typeMenuPick);
    TOGGLE_ITEM(typeMenuItems->items[1], "Bifurcation", BIFURCATION, typeMenuPick);

    XtManageChildren(typeMenuItems->items, 2);

    return pulldown;
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the pulldown menu bar and its menus.
//
Widget
buildMenu(Widget parent)
//
////////////////////////////////////////////////////////////////////////
{
    Widget  menuButtons[6];
    Widget  pulldown1, pulldown3, pulldown4, pulldown5, pulldown6, pulldown7;
    Arg     args[8];
    int        n, m;

// menu bar
    Widget menubar = XmCreateMenuBar(parent, "menuBar", NULL, 0);
    XtVaSetValues (menubar, XmNshadowThickness, 1, NULL);
    pulldown1 = buildFileMenu(menubar);
    pulldown3 = buildStyleMenu(menubar);
    pulldown4 = buildTypeMenu(menubar);
    pulldown7 = buildCoordMenu(menubar);
    pulldown5 = buildOptionMenu(menubar);
    pulldown6 = buildHelpMenu(menubar);

    n = 0; m=0;
    XtSetArg(args[n], XmNsubMenuId, pulldown1); n++;
    menuButtons[m] = XtCreateManagedWidget("File",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown4); n++;
    menuButtons[m] = XtCreateManagedWidget("Type",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown3); n++;
    menuButtons[m] = XtCreateManagedWidget("Style",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown7); n++;
    menuButtons[m] = XtCreateManagedWidget("Draw Coord",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown5); n++;
    menuButtons[m] = XtCreateManagedWidget("Options",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, pulldown6); n++;
    XtSetArg(args[n], XmNmenuHelpWidget, pulldown6); n++;
    menuButtons[m] = XtCreateManagedWidget("Help",
        xmCascadeButtonGadgetClass,
        menubar, args, n);
    ++m;

    XtManageChildren(menuButtons, m);

    return menubar;
}


////////////////////////////////////////////////////////////////////////
//
void 
dimensionToggledCB(Widget w, XtPointer client_data, XtPointer cbs)
//
////////////////////////////////////////////////////////////////////////
{
    static bool buttonState = setShow3D;
    SoSeparator * scene = (SoSeparator *) client_data;
    buttonState = !buttonState;
    if(buttonState)
    {
        setShow3D = true;
        XtSetSensitive (zAxisList, true);
        setListValue();
        XmString xString = XmStringCreateLocalized("3D");
        XtVaSetValues (w, XmNlabelString, xString, NULL);
        XmStringFree(xString);
    }
    else
    {
        setShow3D = false;
        for(int i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;
        zCoordIdxSize = 1;
        XtSetSensitive (zAxisList, false);
        XmString xString = XmStringCreateLocalized("2D");
        XtVaSetValues (w, XmNlabelString, xString, NULL);
        XmStringFree(xString);
    }
    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void 
createBdBoxCB(Widget w, XtPointer client_data, XtPointer cbs)
//
////////////////////////////////////////////////////////////////////////
{
    static bool btnState = false;
    SoSeparator * scene = (SoSeparator *) client_data;
    btnState = !btnState;
    if(btnState)
        scene->addChild(createBoundingBox());
    else
    {
        SoSeparator * bdBox = (SoSeparator *)SoNode::getByName("bdBox");
        scene->removeChild(bdBox);
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the main window contents. In this case, we have a
//  menubar at the top of the window, and a render area filling out
//  the remainder. These widgets are layed out with a Motif form widget.
//
SoXtRenderArea *
buildMainWindow(Widget parent, SoSeparator *sceneGraph)
//
////////////////////////////////////////////////////////////////////////
{
#ifdef USE_EXAM_VIEWER
    SoXtExaminerViewer *renderArea;
#else
    SoXtRenderArea *renderArea;
#endif

    Arg  args[15];
    int  n,i;

    topform = XtCreateWidget("topform", xmFormWidgetClass, parent,NULL, 0);
    Widget menubar = buildMenu(topform);

    Widget listCarrier= XtCreateWidget("ListCarrier",
        xmFormWidgetClass, topform, NULL, 0);

    int nItems = (whichType != BIFURCATION) ? mySolNode.nar : myBifNode.nar;
    int count = XtNumber (xAxis);
    xList = (XmStringTable) XtMalloc(count * sizeof (XmString *));
    for (i = 0; i < count; i++)
        xList[i] = XmStringCreateLocalized (xAxis[i]);

    xAxisList=XtVaCreateManagedWidget ("xAxis",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              xList,
        XmNselectedPosition,   xCoordIndices[0],
        XmNcolumns,            2, 
        XmNmarginHeight,       1,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

    XtAddCallback (xAxisList, XmNselectionCallback,
        xListCallBack, NULL);


    count = XtNumber (yAxis);
    yList = (XmStringTable) XtMalloc(count * sizeof (XmString *));
    for (i = 0; i < count; i++)
        yList[i] = XmStringCreateLocalized (yAxis[i]);

    yAxisList=XtVaCreateManagedWidget ("yAxis",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              yList,
        XmNselectedPosition,   yCoordIndices[0],
        XmNcolumns,            2,
        XmNmarginHeight,       1,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

    XtAddCallback (yAxisList, XmNselectionCallback,
        yListCallBack, NULL);

    count = XtNumber (zAxis);
    zList = (XmStringTable) XtMalloc(count * sizeof (XmString *));
    for (i = 0; i < count; i++)
        zList[i] = XmStringCreateLocalized (zAxis[i]);

    zAxisList=XtVaCreateManagedWidget ("zAxis",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              zList,
        XmNselectedPosition,   zCoordIndices[0],
        XmNcolumns,            2,
        XmNmarginHeight,       1,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

    XtAddCallback (zAxisList, XmNselectionCallback,
        zListCallBack, NULL);

    nItems = (whichType != BIFURCATION) ? 
	               mySolNode.totalLabels+SP_LBL_ITEMS : 
	               myBifNode.totalLabels+SP_LBL_ITEMS;
    count = XtNumber (labels);
    lblList = (XmStringTable) XtMalloc(count * sizeof (XmString *));
    for (i = 0; i < count; i++)
        lblList[i] = XmStringCreateLocalized (labels[i]);

    labelsList=XtVaCreateManagedWidget ("Labels",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              lblList,
        XmNcolumns,            6,
        XmNmarginHeight,       1,
        XmNselectedPosition,   lblChoice[0]+LBL_OFFSET-1,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

    for (i = 0; i < count; i++)
        XmStringFree(lblList[i]);

    XtAddCallback (labelsList, XmNselectionCallback,
        lblListCallBack, NULL);

#ifdef USE_BK_COLOR
    XtVaSetValues (labelsList, XtVaTypedArg,
        XmNbackground, XmRString, "white", 6,
        NULL);
    XtVaSetValues (labelsList, XtVaTypedArg,
        XmNforeground, XmRString, "red", 4,
        NULL);
#endif

    nItems = (whichType != BIFURCATION) ? 
	              mySolNode.nar+mySolNode.npar+specialColorItems : 
				  myBifNode.nar+specialColorItems;
    count = XtNumber (coloringMethodList);
    clrMethodList = (XmStringTable) XtMalloc(count * sizeof (XmString *));

    for (i = 0; i < count; ++i)
        clrMethodList[i] = XmStringCreateLocalized (coloringMethodList[i]);

    colorMethodSeletionList=XtVaCreateManagedWidget ("coloringMethodlist",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              clrMethodList,
        XmNcolumns,            5,
        XmNmarginHeight,       1,
        XmNselectedPosition,   coloringMethod+specialColorItems,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

    XtAddCallback (colorMethodSeletionList, XmNselectionCallback,
        colorMethodSelectionCB, NULL);

//-----------------------------------------------------Nov 06
// build the numPeriodAnimated drop down list
    nItems = 7;
    count = nItems;
    char (*numberPList)[5] = new char [count][5];
    XmStringTable numPList = (XmStringTable) XtMalloc(count * sizeof (XmString *));

    int iam = 1;
    sprintf(numberPList[0], "%i", 0);
    for (i = 0; i < count-2; ++i)
    {
        sprintf(numberPList[i+1], "%i", iam);
        iam *= 2;
    }
    sprintf(numberPList[count-1], "%s", "inf");

    for (i = 0; i < count; ++i)
        numPList[i] = XmStringCreateLocalized (numberPList[i]);

    i = (numPeriodAnimated == 0) ? 0 : (int)((floor(numPeriodAnimated/2.0)))+1;
    Widget numPeriodAnimatedList=XtVaCreateManagedWidget ("list",
        xmComboBoxWidgetClass, listCarrier,
        XmNcomboBoxType,       XmDROP_DOWN_COMBO_BOX,
        XmNitemCount,          nItems,
        XmNitems,              numPList,
        XmNcolumns,            3,
        XmNmarginHeight,       1,
        XmNselectedPosition,   i,
        XmNpositionMode,       XmZERO_BASED,
        NULL);

// Add Callback function for the numberPeriodAnimated drop down list
    XtAddCallback (numPeriodAnimatedList, XmNselectionCallback,
        numPeriodAnimatedCB, NULL);

    for (i = 0; i < count; i++)
        XmStringFree(numPList[i]);
    XtFree((char *)numPList);
//    delete []numberPList;
//----------------------------------------------------------------> Nov 06 End

    Widget xLbl = XtVaCreateManagedWidget("X",xmLabelWidgetClass, listCarrier);
    Widget yLbl = XtVaCreateManagedWidget("Y",xmLabelWidgetClass, listCarrier);
    Widget zLbl = XtVaCreateManagedWidget("Z",xmLabelWidgetClass, listCarrier);
    Widget lLbl = XtVaCreateManagedWidget("Label",xmLabelWidgetClass, listCarrier);
    Widget colorLbl = XtVaCreateManagedWidget("Color",xmLabelWidgetClass, listCarrier);
    Widget numPeriodLbl = XtVaCreateManagedWidget("Period",xmLabelWidgetClass, listCarrier);

    Widget orbitSldLbl = XtVaCreateManagedWidget("Orbit",xmLabelWidgetClass, listCarrier);
    Widget satSldLbl = XtVaCreateManagedWidget("Anim",xmLabelWidgetClass, listCarrier);
    Widget spLbl   = XtVaCreateManagedWidget("   Line  ",xmLabelWidgetClass, listCarrier);
    Widget spLbl2  = XtVaCreateManagedWidget("Thickness",xmLabelWidgetClass, listCarrier);

    n = 0;
    XtSetArg(args[n], XmNminimum, MIN_SAT_SPEED); n++;
    XtSetArg(args[n], XmNmaximum, MAX_SAT_SPEED); n++;
    XtSetArg(args[n], XmNvalue, satSpeed*100);    n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    if(options[OPT_SAT_ANI])
    {
        XtSetArg(args[n], XmNeditable, TRUE); n++;
    }
    else
    {
        XtSetArg(args[n], XmNeditable, FALSE); n++;
    }


    satAniSpeedSlider =
        XtCreateWidget("Speed", xmScaleWidgetClass, listCarrier, args, n);

    XtAddCallback(satAniSpeedSlider, XmNvalueChangedCallback,
        satSpeedCB, NULL);

    n = 0;
    XtSetArg(args[n], XmNminimum, MIN_ORBIT_SPEED); n++;
    XtSetArg(args[n], XmNmaximum, MAX_ORBIT_SPEED); n++;
    XtSetArg(args[n], XmNvalue, orbitSpeed*50);n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    if(options[OPT_PERIOD_ANI])
    {
        XtSetArg(args[n], XmNeditable, TRUE); n++;
    }
    else
    {
        XtSetArg(args[n], XmNeditable, FALSE); n++;
    }
    orbitAniSpeedSlider =
        XtCreateWidget("Speed2", xmScaleWidgetClass, listCarrier, args, n);

    XtAddCallback(orbitAniSpeedSlider, XmNvalueChangedCallback,
        orbitSpeedCB, NULL);

    n = 0;
    XtSetArg (args[n], XmNspinBoxChildType, XmNUMERIC); n++;
    XtSetArg (args[n], XmNminimumValue, 10); n++;
    XtSetArg (args[n], XmNmaximumValue, 100); n++;
    XtSetArg (args[n], XmNdecimalPoints, 1); n++;
    XtSetArg (args[n], XmNincrementValue, 1); n++;
    XtSetArg (args[n], XmNeditable, TRUE); n++;
    XtSetArg (args[n], XmNpositionType, XmPOSITION_VALUE); n++;
    XtSetArg (args[n], XmNposition, lineWidthScaler+10); n++;
    XtSetArg (args[n], XmNcolumns,  3); n++;
    XtSetArg (args[n], XmNarrowSize, 12); n++;
    XtSetArg (args[n], XmNwrap, FALSE); n++;
    Widget spinBox = XmCreateSimpleSpinBox (listCarrier, "spinBox", args, n);

    XtAddCallback(spinBox, XmNvalueChangedCallback,
        lineWidthCB, (XtPointer)spinBox);

#ifdef USE_EXAM_VIEWER
    renderArea = new SoXtExaminerViewer(topform);
#else
    renderArea = new SoXtRenderArea(topform);
#endif

    renderArea->setSize(SbVec2s(winWidth, winHeight));
    renderArea->setBackgroundColor(envColors[0]);

#ifdef USE_EXAM_VIEWER
    n = 0;
    Widget newButton =  XmCreatePushButton(renderArea->getAppPushButtonParent(), "BOX", NULL, 0);
    XtAddCallback(newButton, XmNactivateCallback, createBdBoxCB, sceneGraph);
    renderArea->addAppPushButton(newButton);

    char xString[5];
    if(setShow3D)
    {
        XtSetSensitive (zAxisList, true);
        strcpy(xString,"3D");
    }
    else
    {
        strcpy(xString,"2D");
        XtSetSensitive (zAxisList, false);
    }

    Widget dimButton =  XmCreatePushButton(renderArea->getAppPushButtonParent(), xString, NULL, 0);
    XtAddCallback(dimButton, XmNactivateCallback, dimensionToggledCB, sceneGraph);
    renderArea->addAppPushButton(dimButton);

    vwrAndScene = new ViewerAndScene;
    vwrAndScene->scene  = renderArea->getSceneGraph();
    vwrAndScene->viewer = renderArea;
#endif

// layout
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset,         3            ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset,      3            ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset,        5            ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE); n++;
    XtSetValues(xLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM  ); n++;
    XtSetArg(args[n], XmNtopOffset,         3              ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM  ); n++;
    XtSetArg(args[n], XmNbottomOffset,      3              ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget,        xLbl           ); n++;
    XtSetArg(args[n], XmNleftOffset,        5              ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE  ); n++;
    XtSetValues(xAxisList, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xLbl                    ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xLbl                    ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        xAxisList               ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(yLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        yLbl                    ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(yAxisList, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xLbl                    ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xLbl                    ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        yAxisList               ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(zLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        zLbl                    ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(zAxisList, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xLbl                    ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xLbl                    ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        zAxisList               ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(lLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        lLbl                    ); n++;
    XtSetArg(args[n], XmNleftOffset,        0                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(labelsList, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xLbl                    ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xLbl                    ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        labelsList              ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(colorLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        colorLbl                ); n++;
    XtSetArg(args[n], XmNleftOffset,        0                       ); n++;
    XtSetValues(colorMethodSeletionList,args,n);
    XtManageChild(colorMethodSeletionList);

//Positioning the label for the number of period seleciton list
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xLbl                    ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xLbl                    ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        colorMethodSeletionList ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(numPeriodLbl, args, n);


// layout the numPeriodAnimatedList on the listCarrier.
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        numPeriodLbl            ); n++;
    XtSetArg(args[n], XmNleftOffset,        0                       ); n++;
    XtSetValues(numPeriodAnimatedList,args,n);
    XtManageChild(numPeriodAnimatedList);

//  spinbox labels and spinboxes
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
//    XtSetArg(args[n], XmNleftWidget,        colorMethodSeletionList ); n++;
    XtSetArg(args[n], XmNleftWidget,        numPeriodAnimatedList   ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetValues(spLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNtopWidget,         spLbl                   ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
//    XtSetArg(args[n], XmNleftWidget,        colorMethodSeletionList ); n++;
    XtSetArg(args[n], XmNleftWidget,        numPeriodAnimatedList   ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetValues(spLbl2, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        spLbl                   ); n++;
    XtSetValues(spinBox, args, n);
    XtManageChild(spinBox);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_OPPOSITE_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList               ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        spinBox                 ); n++;
    XtSetArg(args[n], XmNleftOffset,        20                      ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE           ); n++;
    XtSetValues(satSldLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList                ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET          ); n++;
    XtSetArg(args[n], XmNleftWidget,        spinBox                  ); n++;
    XtSetArg(args[n], XmNleftOffset,        20                       ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_NONE            ); n++;
    XtSetValues(orbitSldLbl, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        satSldLbl               ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetValues(satAniSpeedSlider, args, n);
    XtManageChild(satAniSpeedSlider);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNtopWidget,         satAniSpeedSlider             ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET         ); n++;
    XtSetArg(args[n], XmNleftWidget,        satSldLbl                 ); n++;
    XtSetArg(args[n], XmNleftOffset,        5                       ); n++;
    XtSetValues(orbitAniSpeedSlider, args, n);
    XtManageChild(orbitAniSpeedSlider);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_NONE); n++;
    XtSetValues(menubar, args, n);

    n = 0;
#ifdef LIST_UNDER_MENUBAR
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         menubar        ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_NONE); n++;
#else
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_NONE); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM); n++;
#endif
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_FORM); n++;
    XtSetValues(listCarrier, args, n);

    n = 0;
#ifdef LIST_UNDER_MENUBAR
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         xAxisList      ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM  ); n++;
#else
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget,         menubar        ); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget,      xAxisList      ); n++;
#endif
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM  ); n++;
    XtSetArg(args[n], XmNrightAttachment,   XmATTACH_FORM  ); n++;
    XtSetValues(renderArea->getWidget(), args, n);

    XtManageChild(menubar);
    XtManageChild(listCarrier);

    updateScene();
    renderArea->setSceneGraph(sceneGraph);
    renderArea->show();

    XtManageChild(topform);

    return renderArea;
}


////////////////////////////////////////////////////////////////////////
//
//     When the line color changed, this function will be raised.
//
void
linePatternToggledCB(Widget w, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmToggleButtonCallbackStruct *cbs = (XmToggleButtonCallbackStruct*) call_data;
    int value;
    int which = (int)client_data;
    int lineNumber=0, columnNumber =0;

    cbs->set == XmSET ? "on" : cbs->set==XmOFF ? "off" : "interminate";
}


////////////////////////////////////////////////////////////////////////
//
//     When the line color changed, this function will be raised.
//
void
lineColorValueChangedCB(Widget w, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmSpinBoxCallbackStruct *sb = (XmSpinBoxCallbackStruct *) call_data;
    int value;
    int which;
    int lineNumber=0, columnNumber =0;

    if (sb->reason == XmCR_OK)
    {
        XtVaGetValues (sb->widget,  XmNuserData, &which,   0);
        XtVaGetValues (sb->widget,  XmNposition, &value,   0);
        lineNumber = which / 3;
        columnNumber = which % 3;
        lineColorTemp[lineNumber][columnNumber] = value/10.0;
    }
}


////////////////////////////////////////////////////////////////////////
//
//     When the line pattern selection changed, this function will be raised.
//
void
linePatternValueChangedCB(Widget w, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmSpinBoxCallbackStruct *sb = (XmSpinBoxCallbackStruct *) call_data;
    int position, which;

    if (sb->reason == XmCR_OK)
    {
        XtVaGetValues (w, XmNposition, &position, NULL);
        XtVaGetValues (w, XmNuserData, &which,   0);
        linePatternTemp[which] = systemLinePatternValue[position];
    }
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the COLOR and LINE preference sheet stuff.
//
Widget
createLineColorAndPatternPrefSheetGuts(Widget parent, char *name, int id)
//
////////////////////////////////////////////////////////////////////////
{
    Widget widgetList[7];
    Arg args[15];
    int n;

    Widget form = XtCreateWidget("", xmFormWidgetClass, parent, NULL, 0);

    n=0;
    XtSetArg(args[n], XmNcolumns, 12); n++;
    widgetList[0] = XtCreateWidget(name,
        xmLabelGadgetClass, form, args, n);
    XtManageChild (widgetList[0]);

    Widget spin = XmCreateSpinBox (form, "spin", NULL, 0);
    XtAddCallback (spin, XmNvalueChangedCallback, lineColorValueChangedCB, NULL);

    n = 0;
    XtSetArg (args[n], XmNspinBoxChildType, XmNUMERIC) ; n++;
    XtSetArg (args[n], XmNcolumns,          3        ) ; n++;
    XtSetArg (args[n], XmNeditable,         FALSE    ) ; n++;
    XtSetArg (args[n], XmNminimumValue,     0        ) ; n++;
    XtSetArg (args[n], XmNmaximumValue,     10       ) ; n++;
    XtSetArg (args[n], XmNdecimalPoints,    1        ) ; n++;
    XtSetArg (args[n], XmNposition, lineColor[id][0]*10) ; n++;
    XtSetArg (args[n], XmNwrap,             TRUE     ) ; n++;
    XtSetArg (args[n], XmNuserData,         id*3     ) ; n++;

    widgetList[1]= XmCreateTextField (spin, "redText", args, n);
    XtManageChild (widgetList[1]);

    n = 0;
    XtSetArg (args[n], XmNspinBoxChildType,XmNUMERIC); n++;
    XtSetArg (args[n], XmNcolumns,         3        ); n++;
    XtSetArg (args[n], XmNeditable,        FALSE    ); n++;
    XtSetArg (args[n], XmNminimumValue,    0        ); n++;
    XtSetArg (args[n], XmNmaximumValue,    10       ); n++;
    XtSetArg (args[n], XmNdecimalPoints,   1        ); n++;
    XtSetArg (args[n], XmNwrap,            TRUE     ); n++;
    XtSetArg (args[n], XmNposition, lineColor[id][1]*10); n++;
    XtSetArg (args[n], XmNuserData,        id*3+1  ); n++;

    widgetList[2]= XmCreateTextField (spin, "greenText", args, n);
    XtManageChild (widgetList[2]);

    n = 0;
    XtSetArg (args[n], XmNspinBoxChildType,XmNUMERIC); n++;
    XtSetArg (args[n], XmNcolumns,         3        ); n++;
    XtSetArg (args[n], XmNeditable,        FALSE    ); n++;
    XtSetArg (args[n], XmNminimumValue,    0        ); n++;
    XtSetArg (args[n], XmNmaximumValue,    10       ); n++;
    XtSetArg (args[n], XmNdecimalPoints,   1        ); n++;
    XtSetArg (args[n], XmNwrap,            TRUE     ); n++;
    XtSetArg (args[n], XmNposition, lineColor[id][2]*10); n++;
    XtSetArg (args[n], XmNuserData,        id*3+2   ); n++;

    widgetList[3]= XmCreateTextField (spin, "blueText", args, n);
    XtManageChild (widgetList[3]);

    int lengthOfSysPatternArray = XtNumber( systemLinePatternLookAndFeel );
    XmStringTable strList = (XmStringTable) XtMalloc (
        (unsigned) lengthOfSysPatternArray * sizeof (XmString *));
    for (int i = 0; i < lengthOfSysPatternArray; i++)
        strList[i] = XmStringCreateLocalized (systemLinePatternLookAndFeel[i]);

    n = 0;
    XtSetArg (args[n], XmNspinBoxChildType,  XmSTRING); n++;
    XtSetArg (args[n], XmNcolumns,           12        ) ; n++;
    XtSetArg (args[n], XmNeditable,          FALSE); n++;
    XtSetArg (args[n], XmNnumValues,         lengthOfSysPatternArray); n++;
    XtSetArg (args[n], XmNvalues,            strList); n++;
    XtSetArg (args[n], XmNwrap, TRUE); n++;
    XtSetArg (args[n], XmNshadowThickness, 0); n++;
    XtSetArg (args[n], XmNuserData,        id   ); n++;

    Widget lpSpinBox = XmCreateSimpleSpinBox (form, "lpsimple", args, n);
    XtAddCallback (lpSpinBox, XmNvalueChangedCallback, linePatternValueChangedCB, NULL);

    for (int i = 0; i < lengthOfSysPatternArray; i++)
        XmStringFree (strList[i]);
    delete strList;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset,     8            ); n++;
    XtSetValues(widgetList[0], args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget,     widgetList[0]  ); n++;
    XtSetArg(args[n], XmNleftOffset,     10             ); n++;
    XtSetValues(spin, args, n);

    n = 0;
    XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset,      10             ); n++;
    XtSetArg(args[n], XmNleftAttachment,   XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget,       spin  ); n++;
    XtSetArg(args[n], XmNleftOffset,       30             ); n++;
    XtSetValues(lpSpinBox, args, n);

    XtManageChild (spin);
    XtManageChild (lpSpinBox);
    XtManageChild (form);

    return form;
}


////////////////////////////////////////////////////////////////////////
//
Widget 
createColorAndLinePrefSheetHeader(Widget parent)
//
////////////////////////////////////////////////////////////////////////
{
    Widget widget;
    Arg args[6];
    int n;

    Widget form = XtCreateWidget("", xmFormWidgetClass,
        parent, NULL, 0);

    n=0;
    XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset,         20           ); n++;
    XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset,      10           ); n++;
    widget = XtCreateWidget("|PT TYPE| RED  GREEN BLUE   |    LINE PATTERN     |",
        xmLabelGadgetClass, form, args, n);

    XtManageChild(widget);

    return form;
}


///////////////////////////////////////////////////////////////////////
//
//  This simply creates the default parts of the pref dialog.
//
void
createLineAttrPrefSheetParts(Widget widgetList[], int &num, Widget form, char** name)
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePatternTemp[i] = linePattern[i];
        linePatternOld[i]  = linePattern[i];
        for(int j=0; j<3; ++j)
        {
            lineColorTemp[i][j] = lineColor[i][j];
            lineColorOld[i][j]  = lineColor[i][j];
        }
    }

    for(int i=0; i<NUM_SP_POINTS; ++i)
        widgetList[num++] = createLineColorAndPatternPrefSheetGuts(form, name[i], i);
}


////////////////////////////////////////////////////////////////////////
//
//  Given a widget list for the preference sheet and it's lenght
//  lay them out one after the other and manage them all. The dialog
//  is them mapped onto the screen.
//
void
layoutLineAttrPartsOnThePrefSheet(Widget widgetList[],
int num, Widget form)
//
////////////////////////////////////////////////////////////////////////
{
    Arg args[12];
    int n;

    n = 0;
    XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment,         XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset,         20             ); n++;
    XtSetValues(widgetList[0], args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment,         XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset,         20             ); n++;
    XtSetArg(args[n], XmNrightAttachment,       XmATTACH_FORM); n++;
    XtSetValues(widgetList[1], args, n);

    for (int i=2; i<num; i++)
    {
        n = 0;
        XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNtopWidget,         widgetList[i-2]); n++;
        if(i%2 == 0)
        {
            XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
        }
        else
        {
            XtSetArg(args[n], XmNrightAttachment,       XmATTACH_FORM); n++;
        }
        if (i == (num - 1) )
        {
            XtSetArg(args[n], XmNbottomAttachment,  XmATTACH_FORM); n++;
        }
        XtSetValues(widgetList[i], args, n);
    }

    XtManageChildren(widgetList, num);
}


////////////////////////////////////////////////////////////////////////
//
void
createPreferShellAndPanedWindow(Widget &dialog_shell, Widget &panedWin)
//
////////////////////////////////////////////////////////////////////////
{
    Arg args[5];
    unsigned char modality = (unsigned char)XmDIALOG_PRIMARY_APPLICATION_MODAL;

    int n=0;
    XtSetArg(args[n], XmNdialogStyle, modality); ++n;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_NONE); ++n;
    XtSetArg(args[n], XmNnoResize, TRUE); ++n; 
    dialog_shell = XmCreateDialogShell(topform, "Preference Dialog", args, n);

    n=0;
    XtSetArg(args[n], XmNsashWidth, 2); ++n;
    XtSetArg(args[n], XmNsashHeight, 2); ++n;
    XtSetArg(args[n], XmNmarginHeight, 12); ++n;
    XtSetArg(args[n], XmNmarginWidth, 12); ++n;
    panedWin = XmCreatePanedWindow(dialog_shell, "pane",args,n);

}


////////////////////////////////////////////////////////////////////////
//
void
createPreferActionFormControls(Widget &parent)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[13];

    Widget saveBtn, closeBtn, applyBtn, cancelBtn;

    n=0;
    Widget form = XmCreateForm(parent, "control form", args, n);

    n=0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNleftOffset, 100); ++n;
    saveBtn = XmCreatePushButton(form, " Save ", args, n);
    XtManageChild (saveBtn);
    XtAddCallback (saveBtn, XmNactivateCallback, savePreferAndUpdateScene, parent );

    n=0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightOffset, 50); ++n;
    cancelBtn = XmCreatePushButton(form, " Cancel ", args, n);
    XtManageChild (cancelBtn);
    XtAddCallback (cancelBtn, XmNactivateCallback, closePreferDialogAndGiveUpChange, parent);

    n=0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
    XtSetArg(args[n], XmNleftWidget, saveBtn); ++n;
    XtSetArg(args[n], XmNleftOffset, 50); ++n;
    closeBtn = XmCreatePushButton(form, " Update ", args, n);
    XtManageChild (closeBtn);
    XtAddCallback (closeBtn, XmNactivateCallback, closePreferDialogAndUpdateScene, parent );

    n=0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
    XtSetArg(args[n], XmNleftWidget, closeBtn); ++n;
    XtSetArg(args[n], XmNleftOffset, 50); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
    XtSetArg(args[n], XmNrightWidget, cancelBtn); ++n;
    XtSetArg(args[n], XmNrightOffset, 50); ++n;
    applyBtn = XmCreatePushButton(form, " Apply ", args, n);
    XtManageChild (applyBtn);
    XtAddCallback (applyBtn, XmNactivateCallback, applyPreferDialogChangeAndUpdateScene, parent );

    XtManageChild (form);
}


////////////////////////////////////////////////////////////////////////
//
Widget
createPreferDefaultPageFrames(Widget parent, char *frameName)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[12];
    Widget frame, label;

    n = 0;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_IN); n++;
    frame = XmCreateFrame(parent, "frame", args, n);

    n = 0;
    XtSetArg(args[n], XmNframeChildType, XmFRAME_TITLE_CHILD); n++;
    XtSetArg(args[n], XmNchildVerticalAlignment, XmALIGNMENT_CENTER); n++;
    label = XmCreateLabelGadget (frame, frameName, args, n);
    XtManageChild (label);
    return frame;
}


////////////////////////////////////////////////////////////////////////
//
void
graphStyleWidgetToggledCB(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct *) call_data;

    if (state->set == XmSET)
        whichStyleTemp = which;
    else
        whichStyleTemp = 0;
}


////////////////////////////////////////////////////////////////////////
//
void
createGraphStyleFrameGuts(Widget frame)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[12];
    Widget label;
    char * graphStyleItems[]=
    {
        "Line Style", "Tube Style" , "Surface Style"
    };

    n = 0;
    XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); ++n;
    XtSetArg (args[n], XmNnumColumns, 4); ++n;
    Widget toggleBox = XmCreateRadioBox(frame, "togglebox", args, n);
    whichStyleOld  = whichStyle;
    whichStyleTemp = whichStyle;

    for (int i = 0; i < XtNumber (graphStyleItems); i++)
    {
        n = 0;
        if (whichStyle == i) XtSetArg(args[n++], XmNset, XmSET);
        Widget w = XmCreateToggleButtonGadget (toggleBox, graphStyleItems[i], args, n);
        XtAddCallback (w, XmNvalueChangedCallback, graphStyleWidgetToggledCB, (XtPointer) i);
        XtManageChild (w);
    }

    XtManageChild (toggleBox);
    XtManageChild (frame);
}


////////////////////////////////////////////////////////////////////////
//
void
graphTypeWidgetToggledCB(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct *) call_data;

    if (state->set == XmSET)
        whichTypeTemp = which;
    else
        whichTypeTemp = 0;
}


////////////////////////////////////////////////////////////////////////
//
void
graphCoordWidgetToggledCB(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    int which = (int) client_data;
    XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct *) call_data;

    if (state->set == XmSET)
        whichCoordTemp = which;
    else
        whichCoordTemp = 0;
}


////////////////////////////////////////////////////////////////////////
//
void
createGraphTypeFrameGuts(Widget frame)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[12];
    Widget label;
    char * graphTypeItems[]={ "Solution Diagram" , "Bifurcation Diagram" };

    n = 0;
    XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); ++n;
    XtSetArg (args[n], XmNnumColumns, 3); ++n;
    XtSetArg (args[n], XmNindicatorType, XmONE_OF_MANY_ROUND); ++n;
    XtSetArg (args[n], XmNindicatorOn, XmINDICATOR_CHECK); ++n;
    Widget toggleBox = XmCreateRadioBox(frame, "radiobox", args, n);

    whichTypeOld  = whichType;
    whichTypeTemp = whichType;

    for (int i = 0; i < XtNumber (graphTypeItems); i++)
    {
        n = 0;
        if (whichType == i) XtSetArg(args[n++], XmNset, XmSET);
        Widget w = XmCreateToggleButtonGadget (toggleBox, graphTypeItems[i], args, n);
        XtAddCallback (w, XmNvalueChangedCallback, graphTypeWidgetToggledCB, (XtPointer) i);
        XtManageChild (w);
    }

    XtManageChild (toggleBox);
    XtManageChild (frame);
}


////////////////////////////////////////////////////////////////////////
//
// callback for all ToggleButtons.
void
defaultGraphWidgetToggledCB( Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    int bit = (int) client_data;
    XmToggleButtonCallbackStruct *toggle_data = (XmToggleButtonCallbackStruct *) call_data;

    if (toggle_data->set == XmSET)
        graphWidgetToggleSetTemp |= (1 << bit);
    else
        graphWidgetToggleSetTemp &= ~(1 << bit);
}


////////////////////////////////////////////////////////////////////////
//
void
createOptionFrameGuts(Widget frame)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[12];
    Widget label;

    n = 0;
    XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); ++n;
    XtSetArg (args[n], XmNnumColumns, 4); ++n;
    Widget toggleBox = XmCreateRowColumn (frame, "togglebox", args, n);

    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        graphWidgetToggleSetOld = graphWidgetToggleSet;
        graphWidgetToggleSetTemp= graphWidgetToggleSet;
        n = 0;
        if (graphWidgetToggleSet & (1<<i)) XtSetArg(args[n++], XmNset, XmSET);
        Widget w = XmCreateToggleButtonGadget (toggleBox, graphWidgetItems[i], args, n);
        XtAddCallback (w, XmNvalueChangedCallback, defaultGraphWidgetToggledCB, (XtPointer) i);
        XtManageChild (w);
    }

    XtManageChild (toggleBox);
    XtManageChild (frame);
}

////////////////////////////////////////////////////////////////////////
//
void
layoutPreferDefaultPageFrames(Widget widgetList[], int num)
//
////////////////////////////////////////////////////////////////////////
{
    Arg args[12];
    int n = 0;

    XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment,       XmATTACH_FORM); n++;

    for (int i=0; i<num; i++)
    {
        int m = n;
        if(i==0)
        {
            XtSetArg(args[m], XmNtopAttachment,     XmATTACH_FORM); m++;
            XtSetArg(args[m], XmNtopOffset,         20             ); m++;
        }
        else
        {
            XtSetArg(args[m], XmNtopAttachment,     XmATTACH_WIDGET); m++;
            XtSetArg(args[m], XmNtopWidget,         widgetList[i-1]); m++;
        }

        if (i == (num - 1) )
        {
            XtSetArg(args[m], XmNbottomAttachment,  XmATTACH_FORM); m++;
        }
        XtSetValues(widgetList[i], args, m);
    }

    XtManageChildren(widgetList, num);
}


////////////////////////////////////////////////////////////////////////
//
void
createGraphCoordPartsFrameGuts(Widget frame)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[12];
    Widget label;
    char *coordItems[]=
    {
        "No Coordinate", "At Origin" ,
        "At Left & Behind", "At Left & Ahead"  
    };

    n = 0;
    XtSetArg (args[n], XmNpacking, XmPACK_COLUMN); ++n;
    XtSetArg (args[n], XmNnumColumns, 4); ++n;
    Widget toggleBox1 = XmCreateRadioBox(frame, "radio", args, n);

    whichCoordOld  = whichCoord;
    whichCoordTemp = whichCoord;

    for (int i = 0; i < XtNumber (coordItems); i++)
    {
        n = 0;
        if (whichCoord == (unsigned long)i) XtSetArg(args[n++], XmNset, XmSET);
        Widget w = XmCreateToggleButtonGadget (toggleBox1, coordItems[i], args, n);
        XtAddCallback (w, XmNvalueChangedCallback, graphCoordWidgetToggledCB, (XtPointer) i);
        XtManageChild (w);
    }

    XtManageChild (toggleBox1);
    XtManageChild (frame);
}


////////////////////////////////////////////////////////////////////////
//
void
createPreferDefaultPages(Widget parent)
//
////////////////////////////////////////////////////////////////////////
{
    Widget frameList[6];
    int num;
    char * frmNames[]=
    {
        "Optional Widgets", "Graph Type", "Graph Style",
        "Coordinate Parts", "Others"
    };

    for(int i=0; i<XtNumber(frmNames); ++i)
        frameList[i] = createPreferDefaultPageFrames(parent, frmNames[i]);
    num = 0;
    createOptionFrameGuts(frameList[num++]);
    createGraphTypeFrameGuts(frameList[num++]);
    createGraphStyleFrameGuts(frameList[num++]);
    createGraphCoordPartsFrameGuts(frameList[num++]);
    layoutPreferDefaultPageFrames(frameList, num);
}


////////////////////////////////////////////////////////////////////////
//
void
createLineAttPages(Widget parent)
//
////////////////////////////////////////////////////////////////////////
{
    char *tabName[] = { "Line Attributes", "Other Preferences" };
    char *names[] =
    {
        "DEFAULTS", "BP (ALG)", "LP (ALG)",
        "HB      ", "UZ     4", "UZ    -4",
        "LP (DIF)", "BP (DIF)", "PD      ",
        "TR TORUS", "EP (NOR)", "MX (ABN)",
        "OTHERS  "
    };
    char *names2[] =
    {
        "Color 1", "Color 2", "Color 3",
        "Color 4", "Color 5", "Color 6",
        "Color 7", "Color 8", "Color 9",
        "Color 10", "Color 11", "Color 12",
        "Color 13"
    };

    Widget widgetList[20];

    int num = 0;
    widgetList[num++] = createColorAndLinePrefSheetHeader(parent);
    widgetList[num++] = createColorAndLinePrefSheetHeader(parent);
    if(coloringMethod == CL_BRANCH_NUMBER)
        createLineAttrPrefSheetParts(widgetList, num, parent, names2);
    else
        createLineAttrPrefSheetParts(widgetList, num, parent, names);

    layoutLineAttrPartsOnThePrefSheet(widgetList, num, parent);
}


///////////////////////////////////////////////////////////////////////
//
void
createPreferNotebookPages(Widget notebook)
//
////////////////////////////////////////////////////////////////////////
{
    Widget pageForm[2], tab, form;
    int n=0;
    char         buffer[32];
    XmString     xms;
    Arg          args[14];
    char *tabName[] = { "Menu Item Preferences", "Line Attributes" };

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 15); ++n;
    XtSetArg(args[n], XmNmarginWidth,  15); ++n;
    pageForm[0] = XmCreateForm(notebook, "page", args, n);

    n = 0;
    XtSetArg (args[n], XmNnotebookChildType, XmMINOR_TAB); n++;
    sprintf (buffer, "%s", tabName[0] );
    tab = XmCreatePushButton (notebook, buffer, args, n);

    createPreferDefaultPages(pageForm[0]);
    XtManageChild (tab);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 15); ++n;
    XtSetArg(args[n], XmNmarginWidth,  15); ++n;
    pageForm[1] = XmCreateForm(notebook, "page", args, n);

    n=0;
    XtSetArg (args[n], XmNnotebookChildType, XmMINOR_TAB); n++;
    sprintf (buffer, "%s", tabName[1] );
    tab = XmCreatePushButton (notebook, buffer, args, n);
    createLineAttPages(pageForm[1]);
    XtManageChild (tab);

    XtManageChildren (pageForm, 2);
}


////////////////////////////////////////////////////////////////////////
//
void
createPreferNotebookAndActionForm(Widget parentPane, Widget &notebook, Widget &actionForm)
//
////////////////////////////////////////////////////////////////////////
{
    int n;
    Arg args[15];

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 15); ++n;
    XtSetArg(args[n], XmNmarginWidth, 10); ++n;
    XtSetArg (args[n], XmNunitType, Xm1000TH_INCHES); n++;
    XtSetArg (args[n], XmNwidth,  8900); n++;
    XtSetArg (args[n], XmNheight, 4800); n++;
    XtSetArg (args[n], XmNbackPagePlacement, XmTOP_RIGHT); n++;
    XtSetArg (args[n], XmNorientation,      XmHORIZONTAL); n++;
    XtSetArg (args[n], XmNresizePolicy,    XmRESIZE_NONE); n++;
    notebook = XmCreateNotebook (parentPane, "Options", args, n);

    n=0;
    XtSetArg (args[n], XmNpaneMinimum, 25); n++;
    XtSetArg (args[n], XmNpaneMaximum, 35); n++;
    XtSetArg (args[n], XmNresizePolicy,    XmRESIZE_NONE); n++;

    actionForm = XmCreateForm(parentPane, "action form", args, n);
}


////////////////////////////////////////////////////////////////////////
//
//  This creates the preference sheet in a separate window. It
//  calls other routines to create the actual content of the sheet.
//
void
createPreferDialog()
//
////////////////////////////////////////////////////////////////////////
{
    static Widget shell;

    {
        Widget notebook, actionForm, tab, panedWin;
        createPreferShellAndPanedWindow(shell, panedWin);
        createPreferNotebookAndActionForm(panedWin, notebook, actionForm);
        createPreferNotebookPages(notebook);
        createPreferActionFormControls(actionForm);

        XtManageChild (notebook);
        XtManageChild (actionForm);
        XtManageChild (panedWin);
    }
    XtManageChild (shell);
}


///////////////////////////////////////////////////////////////////
//
void
closePreferDialogAndGiveUpChange(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    Widget form = (Widget) client_data;
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePatternTemp[i] = linePatternOld[i];
        linePattern[i]     = linePatternOld[i];
        for(int j=0; j<3; ++j)
        {
            lineColorTemp[i][j] = lineColorOld[i][j];
            lineColor[i][j]     = lineColorOld[i][j];
        }
    }

    whichType     = whichTypeOld;
    whichTypeTemp = whichTypeOld;
    typeMenuItems->which            = whichTypeOld;

    whichStyle     = whichStyleOld;
    whichStyleTemp = whichStyleOld;
    styleMenuItems->which            = whichStyleOld;

    whichCoord     = whichCoordOld;
    whichCoordTemp = whichCoordOld;
    coordMenuItems->which           = whichCoordOld;

    graphWidgetToggleSetTemp = graphWidgetToggleSetOld;
    graphWidgetToggleSet     = graphWidgetToggleSetOld;
    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetOld & (1<<i)) ? true : false;
    }

    if(options[OPT_SAT_ANI])
        XtVaSetValues (satAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (satAniSpeedSlider, XmNeditable, FALSE, NULL);

    if(options[OPT_PERIOD_ANI])
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, TRUE, NULL);
    }
    else
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, FALSE, NULL);
    }

    XtDestroyWidget (XtParent(XtParent(form)));
    updateScene();
}


///////////////////////////////////////////////////////////////////
//
void
closePreferDialogAndUpdateScene(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    Widget form = (Widget) client_data;
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]    = linePatternTemp[i];
        linePatternOld[i] = linePatternTemp[i];
        for(int j=0; j<3; ++j)
        {
            lineColor[i][j]    = lineColorTemp[i][j];
            lineColorOld[i][j] = lineColorTemp[i][j];
        }
    }

    whichType    = whichTypeTemp;
    whichTypeOld = whichTypeTemp;
    typeMenuItems->which           = whichTypeTemp;

    whichStyle    = whichStyleTemp;
    whichStyleOld = whichStyleTemp;
    styleMenuItems->which           = whichStyleTemp;

    whichCoord = whichCoordTemp;
    whichCoordOld = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet    = graphWidgetToggleSetTemp;
    graphWidgetToggleSetOld = graphWidgetToggleSetTemp;
    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    if(options[OPT_SAT_ANI])
        XtVaSetValues (satAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (satAniSpeedSlider, XmNeditable, FALSE, NULL);

    if(options[3])
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, TRUE, NULL);
    }
    else
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, FALSE, NULL);
    }

    updateScene();

    XtDestroyWidget (XtParent(XtParent(form)));
}


///////////////////////////////////////////////////////////////////
//
void
savePreferAndUpdateScene(Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    Widget form = (Widget) client_data;
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]    = linePatternTemp[i];
        linePatternOld[i] = linePatternTemp[i];
        for(int j=0; j<3; ++j)
        {
            lineColor[i][j]    = lineColorTemp[i][j];
            lineColorOld[i][j] = lineColorTemp[i][j];
        }
    }

    whichType    = whichTypeTemp;
    whichTypeOld = whichTypeTemp;
    typeMenuItems->which           = whichTypeTemp;

    whichStyle    = whichStyleTemp;
    whichStyleOld = whichStyleTemp;
    styleMenuItems->which           = whichStyleTemp;

    whichCoord = whichCoordTemp;
    whichCoordOld = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet    = graphWidgetToggleSetTemp;
    graphWidgetToggleSetOld = graphWidgetToggleSetTemp;
    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    if(options[OPT_SAT_ANI])
        XtVaSetValues (satAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (satAniSpeedSlider, XmNeditable, FALSE, NULL);

    if(options[3])
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, TRUE, NULL);
    }
    else
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, FALSE, NULL);
    }

    updateScene();

    writePreferValuesToFile();
    XtDestroyWidget (XtParent(XtParent(form)));
}


///////////////////////////////////////////////////////////////////
//
void
applyPreferDialogChangeAndUpdateScene(
Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePattern[i]=linePatternTemp[i];
        for(int j=0; j<3; ++j)
            lineColor[i][j]=lineColorTemp[i][j];
    }

    whichType = whichTypeTemp;
    typeMenuItems->which        = whichTypeTemp;

    whichStyle = whichStyleTemp;
    styleMenuItems->which        = whichStyleTemp;

    whichCoord = whichCoordTemp;
    coordMenuItems->which        = whichCoordTemp;

    graphWidgetToggleSet = graphWidgetToggleSetTemp;
    for (int i = 0; i < XtNumber (graphWidgetItems); i++)
    {
        options[i] = (graphWidgetToggleSetTemp & (1<<i)) ? true : false;
    }

    if(options[OPT_SAT_ANI])
        XtVaSetValues (satAniSpeedSlider, XmNeditable, TRUE, NULL);
    else
        XtVaSetValues (satAniSpeedSlider, XmNeditable, FALSE, NULL);

    if(options[OPT_PERIOD_ANI])
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, TRUE, NULL);
    }
    else
    {
        XtVaSetValues (orbitAniSpeedSlider, XmNeditable, FALSE, NULL);
    }

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void
writeToFile(char * fileName)
//
////////////////////////////////////////////////////////////////////////
{
    SoWriteAction myWrite;
    myWrite.getOutput()->openFile(fileName);
    myWrite.getOutput()->setBinary(FALSE);
    myWrite.apply(root);
    myWrite.getOutput()->closeFile();
}


////////////////////////////////////////////////////////////////////////
//
//        This routine is called to get a file name using the
// standard file dialog.
//
void
getFileName(int fileMode)
//
////////////////////////////////////////////////////////////////////////
{
    if (fileDialog == NULL)
    {
        Arg args[5];
        int n = 0;
        XtSetArg(args[n], XmNautoUnmanage, TRUE); n++;
        if(topform== NULL)
        {
            printf("mainWindow is NULL!\n");
            return;
        }
        fileDialog = XmCreateFileSelectionDialog(
            XtParent(topform), "File Dialog", args, n);
        XtAddCallback(fileDialog, XmNokCallback,
            fileDialogCB, (XtPointer)fileMode);
    }
    XtManageChild(fileDialog);
}


////////////////////////////////////////////////////////////////////////
//
//        Motif file dialog callback.
//
void
fileDialogCB(Widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    int fileMode = (int)client_data;
    char *filename;
    XmFileSelectionBoxCallbackStruct *data =
        (XmFileSelectionBoxCallbackStruct *)call_data;
    XmStringGetLtoR(data->value,
        (XmStringCharSet) XmSTRING_DEFAULT_CHARSET, &filename);

    SbBool okFile = TRUE;
    if(fileMode == SAVE_ITEM)
        writeToFile(filename);
    else if(fileMode == OPEN_ITEM)
    {
        deleteScene();
        okFile = readFile(filename);
    }

    XtFree(filename);
}


////////////////////////////////////////////////////////////////////////
//
static void
processPrinting(char* filename )
//
////////////////////////////////////////////////////////////////////////
{
    FILE *myFile = fopen(filename, "w");

    if (myFile == NULL)
    {
        fprintf(stderr, "Cannot open output file\n");
        exit(1);
    }

    printf("Printing scene... ");
    fflush(stdout);

    fclose(myFile);

    printf("  ...done printing.\n");
    fflush(stdout);
}


////////////////////////////////////////////////////////////////////////
//
//        Brings up the "HELP INFORMATIO"
//
void
showHelpDialog()
//
////////////////////////////////////////////////////////////////////////
{

    char command[256];
    strcpy(command, autoDir);
    strcat(command,"/plaut04/doc/userguide.pdf");
    if (access(command, R_OK) != 0)
    {
        system("xmessage 'Sorry, could not find "
            "userguide.pdf' > /dev/null");
        return;
    }

    sprintf(command, "which xpdf> /dev/null");

    int err = system(command);
    if (err)
    {
        system("xmessage 'You must install xpdf"
            " for this function to work' > /dev/null");
        return;
    }

    strcpy(command, "xpdf  ");
    strcat(command, autoDir);
    strcat(command, "/plaut04/doc/userguide.pdf & ");
    system(command);
}


////////////////////////////////////////////////////////////////////////
//
//        Brings up the "ABOUT" dialog
//
void
showAboutDialog()
//
////////////////////////////////////////////////////////////////////////
{
    static Widget dialog = (Widget) 0 ;
    XmString      t;
    void          showAboutCB(Widget, XtPointer, XtPointer);
    unsigned char modality = (unsigned char)XmDIALOG_FULL_APPLICATION_MODAL;
    char str[600];
    strcpy(str,"  AUTO plaut04\n\n");
    strcat(str,"  Zhang, Chenghai, Dr. Eusebius J. Doedel\n\n ");
    strcat(str,"  Computer Science Department\n");
    strcat(str,"  Concordia University\n\n");
    strcat(str,"  Montreal, Quebec\n");
    strcat(str,"  CANADA\n\n");
    strcat(str,"  August, 2004 \n");


    if (!dialog)
    {
        Arg args[5];
        int n = 0;
        XmString ok = XmStringCreateLocalized ("OK");
        XtSetArg(args[n], XmNautoUnmanage, False); n++;
        XtSetArg(args[n], XmNcancelLabelString, ok); n++;
        dialog = XmCreateInformationDialog (topform, "About", args, n);
        Widget remove;
        remove = XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON);
        XtUnmanageChild(remove);
        XtAddCallback (dialog, XmNcancelCallback, showAboutCB, NULL);
        XtUnmanageChild (XtNameToWidget (dialog, "OK"));
    }


    t = XmStringCreateLocalized (str);
    XtVaSetValues (dialog, XmNmessageString, t, XmNdialogStyle, modality, NULL);
    XmStringFree (t);
    XtManageChild (dialog);
}


////////////////////////////////////////////////////////////////////////
//
static void 
xListCallBack(Widget combo, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{

    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)call_data;
    int choice = (int) cbs->item_position;
    char *manyChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);

    if (cbs->reason == XmCR_SELECT && cbs->event != NULL)
    {
        int i = 0;
        for(i=0; i<MAX_LIST; i++)
            xCoordIndices[i] = -1;

        i = 0;
        char * tmp;
        tmp = strtok(manyChoice, ",");
        do
        {
            xCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
            tmp = strtok(NULL,",");
        }while(tmp != NULL && i < MAX_LIST); 
        xCoordIdxSize = i;
        updateScene();
    }
}


////////////////////////////////////////////////////////////////////////
//
static void 
yListCallBack(Widget combo, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{

    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)call_data;
    int choice = (int) cbs->item_position;
    char *manyChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);

    if (cbs->reason == XmCR_SELECT && cbs->event != NULL)
    {
        int i = 0;
        for(i=0; i<MAX_LIST; i++)
            yCoordIndices[i] = -1;

        i = 0;
        char * tmp;
        tmp = strtok(manyChoice, ",");
        do
        {
            yCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
            tmp = strtok(NULL,",");
        }while(tmp != NULL && i < MAX_LIST);
        yCoordIdxSize = i;
        updateScene();
    }
}


////////////////////////////////////////////////////////////////////////
//
static void 
zListCallBack(Widget combo, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)call_data;
    int choice = (int) cbs->item_position;
    char *manyChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);

    if (cbs->reason == XmCR_SELECT && cbs->event != NULL)
    {
        int i = 0;
        for(i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;

        i = 0;
        char * tmp;
        tmp = strtok(manyChoice, ",");
        do
        {
            zCoordIndices[i++] = (strcasecmp(tmp,"t")==0) ? 0 : atoi(tmp);
            tmp = strtok(NULL,",");
        }while(tmp != NULL && i < MAX_LIST);
        zCoordIdxSize = i;
        updateScene();
    }
}


////////////////////////////////////////////////////////////////////////
//
static void
lblListCallBack(Widget combo, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmComboBoxCallbackStruct *cbs = (XmComboBoxCallbackStruct *)call_data;
    int choice = (int) cbs->item_position;
    char *manyChoice = (char *) XmStringUnparse (cbs->item_or_text, XmFONTLIST_DEFAULT_TAG,
        XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);

    int i = 0;
    char * tmp;
    static int half = 2;
    tmp = strtok(manyChoice, ",");
    if(choice == 0)
    {
        do
        {
            lblIndices[i++] = (strcasecmp(tmp,"all")==0) ? 0 : atoi(tmp)-myLabels[1]+1;
            tmp = strtok(NULL,",");
        }while(tmp != NULL && i < MAX_LABEL);
        half = 2;
    }
    else if(choice == 1) 
    {
        int j = 1;
        do
        {
            if(clientData.labelIndex[j][2]!= 4 || j%half == 0)
                lblIndices[i++] = j;
            j++;
        } while( j < numLabels-2 );

        half *= 2;
    }
    else if(choice == 2)
    {
        int j = 1;
        do
        {
            if(clientData.labelIndex[j][2] !=  TYPE_UZ  && clientData.labelIndex[j][2] != TYPE_RG) // &&
            //    clientData.labelIndex[j][2] != TYPE_EP_ODE && clientData.labelIndex[j][2] != TYPE_MX)
                lblIndices[i++] = j;
            j++;
        } while( j < numLabels-2 );
        half = 2;
    }
    else if(choice == 3)
    {
        lblIndices[i++] = numLabels;
        half = 2;
    }
    else
    {
        lblIndices[i++] = choice-3;
        half = 2;
    }
    lblIdxSize = i;

    updateScene();
}


////////////////////////////////////////////////////////////////////////
//
void 
updateScene()
//
////////////////////////////////////////////////////////////////////////
{
    int varIndices[3];

    SoSeparator * newScene = new SoSeparator;

    int mx = max(max(xCoordIdxSize, yCoordIdxSize), max(yCoordIdxSize, zCoordIdxSize));
    maxComponent = mx;

    // look for the maximum/minum value in the x-axis, y-axis, z-axis
    if(whichType != BIFURCATION)
    {
        for(int i=0; i<mx; i++)
        {
            int component = i+1;
            varIndices[0]=xCoordIndices[(i>=xCoordIdxSize)?(i%xCoordIdxSize):(i)];
            varIndices[1]=yCoordIndices[(i>=yCoordIdxSize)?(i%yCoordIdxSize):(i)];
            varIndices[2]=zCoordIndices[(i>=zCoordIdxSize)?(i%zCoordIdxSize):(i)];
            searchForMaxMin(component, varIndices);
        }
    }
 
    time_on = TIME_IS_OFF;

    for(int i=0; i<mx; i++)
    {
        curComponent = i+1;
        varIndices[0]=xCoordIndices[(i>=xCoordIdxSize)?(i%xCoordIdxSize):(i)];
        varIndices[1]=yCoordIndices[(i>=yCoordIdxSize)?(i%yCoordIdxSize):(i)];
        varIndices[2]=zCoordIndices[(i>=zCoordIdxSize)?(i%zCoordIdxSize):(i)];

        if (varIndices[0] == 0) time_on = TIME_ON_X; 
        if (varIndices[1] == 0) time_on = TIME_ON_Y; 
        if (varIndices[2] == 0) time_on = TIME_ON_Z; 

        if(whichType != BIFURCATION)
        {
            animationLabel = myLabels[lblIndices[0]];
            copySolDataToWorkArray(varIndices);
            newScene->addChild( createSolutionSceneWithWidgets() );
        }
        else 
        {
            copyBifDataToWorkArray(varIndices);
            newScene->addChild(createBifurcationScene());
        }
    }

    if(userScene == NULL)
    {
        userScene = newScene;
        root->addChild(userScene);
    }
    else
    {
        root->removeAllChildren();
        root->addChild(newScene);
        userScene = newScene;
    }
    iiii++;
}

////////////////////////////////////////////////////////////////////
//
// Routine to create a scene graph representing an auto solution
//
SoSeparator *
createSolutionSceneWithWidgets()
//
////////////////////////////////////////////////////////////////////
{
    float dis = max(max((mySolNode.max[0]-mySolNode.min[0]),
        (mySolNode.max[1]-mySolNode.min[1])),
        (mySolNode.max[2]-mySolNode.min[2]));

    SoSeparator *result = new SoSeparator;

    if(options[OPT_NORMALIZE_DATA])
    {
        normalizeSolData();
    }

    char txtureFileName[256];

    strcpy(txtureFileName, autoDir);
    strcat(txtureFileName,"/widgets/large.rgb");

    if(whichCoord != NO_COORD)
    {
        int type = 0;
        if(whichCoord==LEFTBACK)
            type = 2;
        else if(whichCoord==LEFTAHEAD)
            type = 1;
        else if (whichCoord==COORDORIGIN)
            type = 0;

        SoSeparator * coordSep = new SoSeparator;

        SoTransform * coordXform = new SoTransform;
        coordXform->rotation.setValue(SbVec3f(1.0, 0.0, 0.0), M_PI_2);

        coordSep->addChild(coordXform);
        static int tickers[3];
        if(blDrawTicker)
        {
            tickers[0]=tickers[1]=tickers[2]=5;
        }
        else
        {
            tickers[0]=tickers[1]=tickers[2]=-1;
        }

        float asMax[3], asMin[3];
        if(options[OPT_NORMALIZE_DATA])
        {
            asMin[0]=mySolNode.min[0]; asMin[1]=mySolNode.min[1]; asMin[2]=mySolNode.min[2];
            if ( time_on != TIME_IS_OFF )
            {
                asMax[0] = asMax[1] = asMax[2] = 1;
            } else {
                asMax[0]=mySolNode.max[0]; asMax[1]=mySolNode.max[1]; asMax[2]=mySolNode.max[2];
            }

        }
        else
        {
            asMax[0] = asMax[1] = asMax[2] = 1;
            asMin[0] = asMin[1] = asMin[2] = -1;
        }

        coordSep->addChild(createCoordinates(setShow3D, type, asMax, asMin, tickers, whichCoord));
        result->addChild(coordSep);
    }

    result->addChild(renderSolution());

    if(iiii && options[OPT_BACKGROUND])
    {
        char bgFileName[256];
        strcpy(bgFileName, autoDir);
        strcat(bgFileName, "/plaut04/widgets/background.rgb");
        result->addChild(drawStarryBackground(bgFileName));
    }

    static SoSeparator *leg = new SoSeparator;
    if(iiii && options[OPT_LEGEND])
    {
        leg = addLegend();
        result->addChild(leg);
    }

    return result;
}


SoSeparator * drawStarryBackground(char * bgFileName)
{
    SoSeparator *starryBg = new SoSeparator;
    SoTexture2 * bgTxture = new SoTexture2;
    bgTxture->filename.setValue(bgFileName);
    SoMaterial *bgMaterial = new SoMaterial;
    bgMaterial->transparency = bgTransparency;
    starryBg->addChild(bgMaterial);
    starryBg->addChild(bgTxture);
    SoTransform *bgXform = new SoTransform;
    bgXform->translation.setValue(0,0,-9);

    SoOrthographicCamera * bgCamera = new SoOrthographicCamera;

    SoCube * bgPlane = new SoCube();
    bgPlane->width  = 3.0;
    bgPlane->height = 3.0;
    bgPlane->depth  = 0.01;

    SoDirectionalLight * bgLight = new SoDirectionalLight;
    bgLight->intensity = 0.05;
    bgLight->direction.setValue(0,0, -10);
    starryBg->addChild(bgLight);
    starryBg->addChild(bgCamera);
    starryBg->addChild(bgXform);
    starryBg->addChild(bgPlane);
    return starryBg;
}


SoSeparator * addLegend()
{
    SoSeparator * result = new SoSeparator;
    SoOrthographicCamera *legendCamera = new SoOrthographicCamera;
    result->addChild(legendCamera);
    SbVec3f lgPos;
    lgPos.setValue(0.85, 0.65, -5.9);

    if(coloringMethod == CL_BRANCH_NUMBER)
    {
        result->addChild(createBranchLegend(lgPos, lineColor));
    }
    else if(coloringMethod == CL_STABILITY)
    {
        SbColor color[2];
        color[0].setValue(1,0,0);
        color[1].setValue(0,0,1);
        result->addChild(createStabilityLegend(lgPos, color));
    }
    else if( coloringMethod == CL_ORBIT_TYPE)
    {
        result->addChild(createDiscreteLegend(lgPos, lineColor));
    }
    else
    {
        double values[5];
        setLegendValues(values);
        result->addChild(createLegend(lgPos, values));
    }
    result->addChild(new SoDirectionalLight);

    return result;
}


////////////////////////////////////////////////////////////////////////
//
void 
setLegendValues(double* values)
//
////////////////////////////////////////////////////////////////////////
{
    for(int i=0; i<5; ++i)
        values[i] = legendScaleValues[0] + i*(legendScaleValues[1]-legendScaleValues[0])/4.0;
}


//////////////////////////////////////////////////////////////////////////
//
// Routine to create a scene graph representing an auto bifurcation
//
SoSeparator *
createBifurcationScene()
//
//////////////////////////////////////////////////////////////////////////
{
    float dis = fabs(max(max((myBifNode.max[0]-myBifNode.min[0]),
        (myBifNode.max[1]-myBifNode.min[1])),
        (myBifNode.max[2]-myBifNode.min[2])));

    SoSeparator *result = new SoSeparator;
    result->ref();

    if(options[OPT_NORMALIZE_DATA])
    {
        normalizeBifData();
    }

    if(whichCoord != NO_COORD)
    {
        int type = 0;
        if(whichCoord==LEFTBACK)
            type = 2;
        else if(whichCoord==LEFTAHEAD)
            type = 1;
        else if (whichCoord==COORDORIGIN)
            type = 0;

        SoSeparator * coordSep = new SoSeparator;

        SoTransform * coordXform = new SoTransform;
        coordXform->rotation.setValue(SbVec3f(1.0, 0.0, 0.0),M_PI_2);

        coordSep->addChild(coordXform);
        static int tickers[3];
        if(blDrawTicker)
        {
            tickers[0]=tickers[1]=tickers[2]=5;
        }
        else
        {
            tickers[0]=tickers[1]=tickers[2]=-1;
        }

        float asMax[3], asMin[3];
        if(options[OPT_NORMALIZE_DATA])
        {
            asMax[0]=myBifNode.max[0]; asMax[1]=myBifNode.max[1];asMax[2]=myBifNode.max[2];
            asMin[0]=myBifNode.min[0]; asMin[1]=myBifNode.min[1];asMin[2]=myBifNode.min[2];
        }
        else
        {
            asMax[0] = asMax[1] = asMax[2] = 1;
            asMin[0] = asMin[1] = asMin[2] = -1;
        }

        coordSep->addChild(createCoordinates(setShow3D, type, asMax, asMin, tickers, whichCoord));

        result->addChild(coordSep);
    }

    SoSeparator * bifBranchSep = renderBifurcation();
    result->addChild(bifBranchSep);

    static int iiii = 0;
    if(iiii && options[OPT_BACKGROUND])
    {
        char bgFileName[256];
        strcpy(bgFileName, autoDir);
        strcat(bgFileName, "/plaut04/widgets/background.rgb");
        result->addChild(drawStarryBackground(bgFileName));
    }

    if(iiii && options[OPT_LEGEND])
    {
        result->addChild(addLegend());
    }
    iiii++;

    return result;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawLabelPtsInBifurcationScene()
//
//////////////////////////////////////////////////////////////////////////
{
    float dis = !options[OPT_NORMALIZE_DATA] ? fabs(max(max((myBifNode.max[0]-myBifNode.min[0]),
        (myBifNode.max[1]-myBifNode.min[1])),
        (myBifNode.max[2]-myBifNode.min[2]))) : 2.0;

    SoSeparator * result = new SoSeparator;
    int lbl = myLabels[lblIndices[0]], row = 0;
    float position[3];
    int lbType;

    if(lbl == 0)
    {
        for(int i=0; i<lblIdxSize; ++i)
        {
            int k = 0;
            int idx = 0;
            do
            {
                SoMaterial *lblMtl;
                idx = (k==0) ? myLabels[k] : myLabels[k]-1;
                row = clientData.labelIndex[idx][1] - 1;
                lbType = clientData.labelIndex[idx][2];
//              lbType = clientData.labelIndex[k][2];
    
                lblMtl = setLabelMaterial(lbType);
                result->addChild(lblMtl);
    
                position[0] = myBifNode.xyzCoords[row][0];
                position[1] = myBifNode.xyzCoords[row][1];
                position[2] = myBifNode.xyzCoords[row][2];

                result->addChild( drawASphere(position, dis*lineWidthScaler*0.005));;
                ++k;
            } while( k < clientData.totalLabels);
        }
    }
    else if(lbl != MY_NONE)
    {
        for(int i=0; i<lblIdxSize; ++i)
        {
            SoMaterial *lblMtl;

            row = clientData.labelIndex[lblIndices[i]-1][1] -1;
            lbType = clientData.labelIndex[lblIndices[i]-1][2];

            lblMtl = setLabelMaterial(lbType);
            result->addChild(lblMtl);

            position[0] = myBifNode.xyzCoords[row][0];
            position[1] = myBifNode.xyzCoords[row][1];
            position[2] = myBifNode.xyzCoords[row][2];

            result->addChild( drawASphere(position, dis*lineWidthScaler*0.005));
        }
    }
    return result;
}


/////////////////////////////////////////////////////////////////
//
//                 Set Material for Labels
//
SoMaterial *
setLabelMaterial(int lblType)
//
/////////////////////////////////////////////////////////////////
{
    SoMaterial *lblMtl = new SoMaterial;
    lblMtl->transparency = 0.0;
    switch(lblType)
    {
        case 0 :
            lblMtl->diffuseColor.setValue(lineColor[0]);
            break;
        case 1 : 
            lblMtl->diffuseColor.setValue(lineColor[1]);
            break;
        case 2 :
            lblMtl->diffuseColor.setValue(lineColor[2]);
            break;
        case 3 :
            lblMtl->diffuseColor.setValue(lineColor[3]);
            break;
        case 4 :
            lblMtl->diffuseColor.setValue(lineColor[4]);
            break;
        case -4 :
            lblMtl->diffuseColor.setValue(lineColor[5]);
            break;
        case 5 : 
            lblMtl->diffuseColor.setValue(lineColor[6]);
            break;
        case 6 : 
            lblMtl->diffuseColor.setValue(lineColor[7]);
            break;
        case 7 :
            lblMtl->diffuseColor.setValue(lineColor[8]);
            break;
        case 8 :
            lblMtl->diffuseColor.setValue(lineColor[9]);
            break;
        case 9 :
            lblMtl->diffuseColor.setValue(lineColor[10]);
            break;
        case -9 :
            lblMtl->diffuseColor.setValue(lineColor[11]);
            break;
        default :
            lblMtl->diffuseColor.setValue(lineColor[12]);
    }
    return lblMtl;
}


/////////////////////////////////////////////////////////////////
//
SoSeparator *
drawABifBranchUsingTubes(int iBranch, long int l,
long int sumX, float scaler, int stability, int type)
//
/////////////////////////////////////////////////////////////////
{
    SoSeparator * tSep = new SoSeparator;
    long int upperlimit = myBifNode.numVerticesEachBranch[l-1];

    if(upperlimit <= 1)
    {
        return tSep;
    }

    float (*path)[3] = new float[upperlimit][3];
    float *colorBase = new float[upperlimit*11];
    Tube tube;

    for(long int i=0; i<upperlimit; i++)
    {
        long int idx = i+sumX;
        path[i][0]=myBifNode.xyzCoords[idx][0];
        path[i][1]=myBifNode.xyzCoords[idx][1];
        path[i][2]=myBifNode.xyzCoords[idx][2];
        if(coloringMethod>=0)
            for(int k=0; k<11; ++k)
                colorBase[i*11+k]  = clientData.bifData[idx][coloringMethod];
        else if(coloringMethod==CL_POINT_NUMBER)
            for(int k=0; k<11; ++k)
                colorBase[i*11+k]  = i;
        else if(coloringMethod == CL_STABILITY)
            for(int k=0; k<11; ++k)
                colorBase[i*11+k] = myBifNode.ptStability[idx];
    }

    if(coloringMethod == CL_BRANCH_NUMBER)
        tSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
    else if(coloringMethod == CL_STABILITY)
        tSep->addChild(setLineColorBlendingByStability(colorBase, upperlimit*11, stability, scaler));
    else if(coloringMethod == CL_ORBIT_TYPE)
        tSep->addChild(setLineAttributesByType(stability, type, scaler));
    else
        tSep->addChild(setLineColorBlending(colorBase,
            upperlimit*11, stability, type, scaler));

    tube = Tube(upperlimit, path, lineWidthScaler*0.005, 10);
    tSep->addChild(tube.createTube());
    delete [] path;
    delete [] colorBase;
    return tSep;
}


/////////////////////////////////////////////////////////////////
//
//                  create bifurcation scene
//
SoSeparator *
renderBifurcation()
//
//////////////////////////////////////////////////////////////////////////
{
//    SoDrawStyle *drawStyle = new SoDrawStyle;
//    drawStyle->lineWidth = 1.0;

    SoSeparator *bifSep = new SoSeparator;
    SoMaterial *bifMtl= new SoMaterial;

    bifMtl->ambientColor.setValue(0.5,0.5,0.5);
    bifMtl->diffuseColor.setValue(0.9,1.0,0.9);
    bifMtl->specularColor.setValue(0.0,0.5,0.5);
    bifMtl->shininess = 0.5;
    bifMtl->transparency = 0.0;

    SoGroup *bifGroup = new SoGroup;

//    SoLineSet *bifLines = new SoLineSet;
//    SoCoordinate3 *bifCoords = new SoCoordinate3;

    if(whichStyle == TUBE )
    {
        long int si = 0, k = 0;
        for(int ka=0; ka<myBifNode.numBranches; ka++)
        {
            k = k+1;
            bifGroup->addChild(drawABifBranchUsingTubes(ka, k, si, 1*lineWidthScaler,
                clientData.labelIndex[k][3], clientData.labelIndex[k][2]));
            si += myBifNode.numVerticesEachBranch[ka];
        }
    }
    else if (whichStyle == NURBS)
    {
        long int si = 0, k = 0;
        for(int ka=0; ka<myBifNode.numBranches; ka++)
        {
            k = k+1;
            bifGroup->addChild(drawABifBranchUsingNurbsCurve(ka, k, si, 1*lineWidthScaler,
                clientData.labelIndex[k][3],
                clientData.labelIndex[k][2]));
            si += myBifNode.numVerticesEachBranch[ka];
        }

    }
    else 
    {
        long int si = 0, k = 0;
        for(int ka=0; ka<myBifNode.numBranches; ka++)
        {
            k = k+1;
            bifGroup->addChild(drawABifBranchUsingLines(ka, k, si, 1*lineWidthScaler,
                clientData.labelIndex[k][3],
                clientData.labelIndex[k][2]));
            si += myBifNode.numVerticesEachBranch[ka];
        }
    }

    if(options[OPT_PERIOD_ANI])
        bifSep->addChild(drawLabelPtsInBifurcationScene());

    bifSep->addChild(bifMtl);
    bifSep->addChild(bifGroup);

    return bifSep;
}


//////////////////////////////////////////////////////////////////////////
//
// Description: Draw the whole solution family using TUBES.
//
SoSeparator *
drawSolUsingTubes()
//
//////////////////////////////////////////////////////////////////////////
{
    SoSeparator * tubeSep = new SoSeparator;

    float dis = (!options[OPT_NORMALIZE_DATA]) ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2;

    int32_t myint[10];
    long int sumX = 0;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
    float scaler = 0.1;

    for(long int j=0; j<mySolNode.numOrbits; ++j)
    {
        if(j >= sumOrbit)
        {
            curBranchID = mySolNode.branchID[++iBranch];
            sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
        }
        long int upperlimit = mySolNode.numVerticesEachPeriod[j];

        if(upperlimit >0 )
        {
            if(upperlimit == 1)
            {
                int idx = sumX;
                SoSeparator * ptSep = new SoSeparator;
                SoSphere * aPoint = new SoSphere;
                aPoint->radius = dis*STATIONARY_POINT_RADIUS;

                int stability=clientData.labelIndex[j+1][3];
                int type =clientData.labelIndex[j+1][2];
                float scaler = lineWidthScaler;

                if(coloringMethod == CL_BRANCH_NUMBER)
                    ptSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
                else if(coloringMethod == CL_STABILITY)
                    ptSep->addChild(setLineAttributesByStability(stability, scaler));
                else if(coloringMethod == CL_ORBIT_TYPE)
                    ptSep->addChild(setLineAttributesByType(stability, type, scaler));
                else if(coloringMethod == CL_LABELS)
                {
                    ptSep->addChild(setLineAttributesByParameterValue(
                        j, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                        stability, scaler));
                }
                else if(coloringMethod == CL_COMPONENT)
                {
                    ptSep->addChild(setLineAttributesByParameterValue(
                        curComponent, maxComponent, maxComponent/2.0, 0,
                        stability, scaler));
                }
                else 
                {
                    SoMaterial * ptMtl = new SoMaterial;
                    ptMtl->diffuseColor.setValue(1,0,0);
                    ptSep->addChild(ptMtl);
                }

        
                float ver[2][3];
        
                if(time_on != TIME_IS_OFF)
                {
                    if(time_on == TIME_ON_X)
                    {
                        ver[0][0] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[0]-mySolNode.min[0]) : 0;
                        ver[0][1] = mySolNode.xyzCoords[idx][1];
                        ver[0][2] = 0;
                        ver[1][0] = (mySolNode.max[0] <= 1.0) ? 1.0 : mySolNode.max[0];
                        ver[1][1] = mySolNode.xyzCoords[idx][1];
                        ver[1][2] = 0;
                    }
                    else if(time_on == TIME_ON_Y)
                    {
                        ver[0][0] = mySolNode.xyzCoords[idx][0];
                        ver[0][1] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[1]-mySolNode.min[1]) : 0;
                        ver[0][2] = 0;
                        ver[1][0] = mySolNode.xyzCoords[idx][0]; 
                        ver[1][1] = ( mySolNode.max[1] <= 1.0 ) ? 1.0 : mySolNode.max[1];
                        ver[1][2] = 0;
                    }
                    else if(time_on == TIME_ON_Z)
                    {
                        ver[0][0] = mySolNode.xyzCoords[idx][0];
                        ver[0][1] = 0; 
                        ver[0][2] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[2]-mySolNode.min[2]) : 0; 
                        ver[1][0] = mySolNode.xyzCoords[idx][0];
                        ver[1][1] = 0; 
                        ver[1][2] = ( mySolNode.max[2] <= 1.0 ) ? 1.0 : mySolNode.max[2]; 
                    }
        
                    SoCoordinate3 *myC = new SoCoordinate3;
                    myC->point.setValues(0, 2, ver);
                    myint[0]=2;
                    SoLineSet *myL = new SoLineSet;
                    myL->numVertices.setValues(0, 1, myint);
                    ptSep->addChild(myC);
                    ptSep->addChild(myL);
                    tubeSep->addChild(ptSep);
                }
                else
                {
                    SoTransform * aTrans = new SoTransform;
                    aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
        		           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
                    ptSep->addChild(aTrans);
                    SoSphere *aPoint = new SoSphere;
                    aPoint->radius = dis * STATIONARY_POINT_RADIUS;
                    ptSep->addChild(aPoint);
                    tubeSep->addChild(ptSep);
                }
/*
                aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
				            mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
                ptSep->addChild(aTrans);
                ptSep->addChild(aPoint);
*/
                tubeSep->addChild(ptSep);
            }
            else
            {

                float (*path)[3] = new float[upperlimit][3];
                float *colorBase = new float[upperlimit*11];
                Tube tube;
                for(int i=0; i<upperlimit; i++)
                {
                    int idx = i+sumX;
                    path[i][0]=mySolNode.xyzCoords[idx][0];
                    path[i][1]=mySolNode.xyzCoords[idx][1];
                    path[i][2]=mySolNode.xyzCoords[idx][2];
                    if(coloringMethod>=0)
                        for(int k=0; k<11; ++k)
                            colorBase[i*11+k]  = clientData.solData[idx][coloringMethod];
                    if(coloringMethod==CL_POINT_NUMBER)
                        for(int k=0; k<11; ++k)
                            colorBase[i*11+k]  = i;

                }
                int stability=clientData.labelIndex[j+1][3];
                int type =clientData.labelIndex[j+1][2];
                float scaler = lineWidthScaler;

//                if(maxComponent == 1)
//                {
                    if(coloringMethod == CL_BRANCH_NUMBER)
                        tubeSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
                    else if(coloringMethod == CL_STABILITY)
                        tubeSep->addChild(setLineAttributesByStability(stability, scaler));
                    else if(coloringMethod == CL_ORBIT_TYPE)
                        tubeSep->addChild(setLineAttributesByType(stability, type, scaler));
                    else if(coloringMethod == CL_LABELS)
                    {
/*
                        double bMin = 0;
                        for(int ib = 0; ib< iBranch; ++ib)
                            bMin +=  mySolNode.numOrbitsInEachBranch[ib];
                        double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
                        tubeSep->addChild(setLineAttributesByParameterValue(
                            j, bMax, (bMax+bMin)/2.0, bMin,
                            stability, scaler));
*/
                        tubeSep->addChild(setLineAttributesByParameterValue(
                            j, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                            stability, scaler));
                    }
                    else if(coloringMethod == CL_COMPONENT)
                        tubeSep->addChild(setLineAttributesByParameterValue(
                                curComponent, maxComponent, maxComponent/2.0, 0,
                                stability, scaler));
                    else if(coloringMethod >= mySolNode.nar)
                        tubeSep->addChild(setLineAttributesByParameterValue(
                                mySolNode.par[j][mySolNode.parID[coloringMethod-mySolNode.nar]],
                                mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                                mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                                mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                                stability, scaler));
                    else
                        tubeSep->addChild(setLineColorBlending(colorBase,
                            upperlimit*11,stability, type, scaler));
//                }
//                else
//                {
//                    tubeSep->addChild(setLineAttributesByParameterValue(
//                        curComponent, maxComponent, maxComponent/2.0, 0,
//                        stability, scaler));
//                }

                tube = Tube(upperlimit, path, lineWidthScaler*0.005, 10);
                tubeSep->addChild(tube.createTube());
                delete [] path;
                delete [] colorBase;
            }
        }
        sumX += upperlimit;
    }
    return tubeSep;
}


//////////////////////////////////////////////////////////////////////////
//
// draw the diagram with surface
//
SoSeparator *
drawASolBranchUsingSurface(long obStart, long obEnd, long numVert)
//
//////////////////////////////////////////////////////////////////////////
{
    float dis = (!options[OPT_NORMALIZE_DATA]) ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0;

    SoSeparator *solSurface = new SoSeparator;
    float (*strip)[3];
    long int npt = numVert;
    long int sum = 0;

    strip = new float[2*npt][3];

    if((obEnd-obStart)>1)
    {
        if(npt>1)
        {
            for(int i=0; i<npt; i++)
            {
                strip[i*2][0] = mySolNode.xyzCoords[i][0];
                strip[i*2][1] = mySolNode.xyzCoords[i][1];
                strip[i*2][2] = mySolNode.xyzCoords[i][2];
            }
        }
        else
        {
            cout <<" Only one point in the period, no surface can be drawn!"<<endl;
            long int idx = obStart;
            SoSeparator * ptSep = new SoSeparator;
            SoTransform * aTrans = new SoTransform;
            aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
			            mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
            ptSep->addChild(aTrans);

            SoSphere *aPoint = new SoSphere;
            aPoint->radius = dis * STATIONARY_POINT_RADIUS;
            ptSep->addChild(aPoint);
            solSurface->addChild(ptSep);
            return solSurface;
        }

        sum += npt;
        for(int j=obStart; j<obEnd; ++j)
        {
            npt= mySolNode.numVerticesEachPeriod[j];
            if(npt>1)
            {
                for(int i=0; i<npt; i++)
                {
                    strip[i*2+1][0] = mySolNode.xyzCoords[i+sum][0];
                    strip[i*2+1][1] = mySolNode.xyzCoords[i+sum][1];
                    strip[i*2+1][2] = mySolNode.xyzCoords[i+sum][2];
                }
                solSurface->addChild(drawAStrip(strip,npt*2));
                for(int i=0; i<npt; i++)
                {
                    strip[i*2][0] = strip[i*2+1][0];
                    strip[i*2][1] = strip[i*2+1][1];
                    strip[i*2][2] = strip[i*2+1][2];
                }
                sum += npt;
            }
            else
            {
                cout <<" Only one point in the period, no surface can be drawn!"<<endl;
                long int idx = sum;
                SoSeparator * ptSep = new SoSeparator;
                SoTransform * aTrans = new SoTransform;
                aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
				           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
                ptSep->addChild(aTrans);

                SoSphere *aPoint = new SoSphere;
                aPoint->radius = dis * STATIONARY_POINT_RADIUS;
                ptSep->addChild(aPoint);
                solSurface->addChild(ptSep);
                return solSurface;

            }
        }
    }
    else
    {
        cout <<" Only one solution! No surface is able to be drawn!"<<endl;
        cout <<" Choose LINE/TUBE to view it."<<endl;
        return NULL;
    }
    delete [] strip;
    return solSurface;
}


///////////////////////////////////////////////////////////////////
//
SoGroup *
renderSolutionTubes()
//
//////////////////////////////////////////////////////////////////////////
{
    SoGroup * solGroup = new SoGroup;
    SoCoordinate3 *solCoords = new SoCoordinate3;

    if(animationLabel == 0)
        solGroup->addChild(drawSolUsingTubes());
    else if(animationLabel != MY_NONE)
    {
        for(int n=0; n<lblIdxSize; ++n)
        {
            animationLabel=myLabels[lblIndices[n]];

            int si = 0, k = 0;
            int iBranch = 0;
            int curBranchID = mySolNode.branchID[iBranch];
            int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];

            for(int ka=0; ka<mySolNode.numOrbits; ka++)
            {
                if(ka >= sumOrbit)
                {
                    curBranchID = mySolNode.branchID[++iBranch];
                    sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
                }
                k = k+1;
                if(myLabels[ka+1]>=animationLabel) break;
                si += mySolNode.numVerticesEachPeriod[ka];
            }

            if(options[OPT_SAT_ANI])
            {
                solGroup->addChild(animateOrbitWithTail(iBranch, k, si));
            }
            else
            {
                solGroup->addChild(drawAnOrbitUsingTubes(iBranch, k, si, 1*lineWidthScaler,
                    clientData.labelIndex[k][3],
                    clientData.labelIndex[k][2]));
            }
        }
    }

    if(options[OPT_PERIOD_ANI])
    {
        bool aniColoring;
        aniColoring = (animationLabel == 0) ? false : true;
        solGroup->addChild(animateSolutionUsingTubes(aniColoring));
    }
    return solGroup;
}


//////////////////////////////////////////////////////////////////////////
//
SoGroup *
renderSolutionSurface()
//
//////////////////////////////////////////////////////////////////////////
{
    SoGroup *solGroup = new SoGroup;
    SoMaterial *solMtl = new SoMaterial;
    solMtl->diffuseColor.setValue(envColors[5]);
    solGroup->addChild(solMtl);
    SoMaterialBinding *myMtlBinding = new SoMaterialBinding;
    myMtlBinding->value= SoMaterialBinding::PER_PART;
    solGroup->addChild(myMtlBinding);

    long start = 0;
    long end = 0;

    int sumOrbit = 0;
    long numVert = 0;
    for(int iBranch = 0; iBranch < mySolNode.numBranches; ++iBranch)
    {
        end += mySolNode.numOrbitsInEachBranch[iBranch];
        numVert = mySolNode.numVerticesEachPeriod[sumOrbit];
        SoSeparator * as = drawASolBranchUsingSurface(start+1, end, numVert);
        if(as !=NULL)
            solGroup->addChild(as);
        start += mySolNode.numOrbitsInEachBranch[iBranch];
        sumOrbit += mySolNode.numOrbitsInEachBranch[iBranch];
    }

    if(options[OPT_PERIOD_ANI])
        solGroup->addChild(animateSolutionUsingTubes(true));

    int si = 0, k = 0;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
    for(int ka=0; ka<mySolNode.numOrbits; ka++)
    {
        if(ka >= sumOrbit)
        {
            curBranchID = mySolNode.branchID[++iBranch];
            sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
        }
        k = k+1;
        if(myLabels[ka+1]>=animationLabel) break;
        si += mySolNode.numVerticesEachPeriod[ka];
    }

    if(options[OPT_SAT_ANI])
        solGroup->addChild(animateOrbitWithTail(iBranch, k, si));
    else
        solGroup->addChild(drawAnOrbitUsingTubes(iBranch, k, si, 1*lineWidthScaler,
            clientData.labelIndex[k][3], clientData.labelIndex[k][2]));
    return solGroup;
}


///////////////////////////////////////////////////////////////////////
//
//           draw the solutions by Mesh points
//
SoGroup *
renderSolutionPoints(int style)
//
//////////////////////////////////////////////////////////////////////////
{
    SoGroup       *solGroup  = new SoGroup;
    SoCoordinate3 *solCoords = new SoCoordinate3;

    if(animationLabel == 0)
    {
        long int si = 0, k = 0;
        int iBranch = 0;
        int curBranchID = mySolNode.branchID[iBranch];
        int sumOrbit = mySolNode.numOrbitsInEachBranch[iBranch];

        for(long int ka=0; ka<mySolNode.numOrbits; ka++)
        {
            if(ka >= sumOrbit)
            {
                curBranchID = mySolNode.branchID[++iBranch];
                sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
            }

            k = k+1;
            solGroup->addChild(drawAnOrbitUsingPoints(style, iBranch, /*curBranchID,*/ k, si, lineWidthScaler,\
                clientData.labelIndex[k][3], clientData.labelIndex[k][2], true));
            si += mySolNode.numVerticesEachPeriod[ka];
        }
    }
    else if(animationLabel != MY_NONE)
    {
        for(int n=0; n<lblIdxSize; ++n)
        {
            animationLabel=myLabels[lblIndices[n]];
            int si = 0, k = 0;
            int iBranch = 0;
            int curBranchID = mySolNode.branchID[iBranch];
            int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
            for(int ka=0; ka<mySolNode.numOrbits; ka++)
            {
                if(ka >= sumOrbit)
                {
                    curBranchID = mySolNode.branchID[++iBranch];
                    sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
                }

                k = k+1;
                if(myLabels[ka+1]>=animationLabel) break;
                si += mySolNode.numVerticesEachPeriod[ka];
            }
            if(options[OPT_SAT_ANI])
            {
                solGroup->addChild(animateOrbitWithTail(iBranch, k, si));
            }
            else
            {
                solGroup->addChild(drawAnOrbitUsingPoints(style, iBranch, k, si, lineWidthScaler,\
                    clientData.labelIndex[k][3], clientData.labelIndex[k][2], true));
            }
        }
    }

    if(options[OPT_PERIOD_ANI])
    {
        bool coloring;
        coloring = (animationLabel == 0) ? false : true;
        solGroup->addChild(animateSolutionUsingPoints(style, coloring));
    }

    return solGroup;
}


///////////////////////////////////////////////////////////////////////
//
//           draw the solutions by lines
//
SoGroup *
renderSolutionLines()
//
//////////////////////////////////////////////////////////////////////////
{
    SoGroup       *solGroup  = new SoGroup;
    SoCoordinate3 *solCoords = new SoCoordinate3;

    if(animationLabel == 0)
    {
        long int si = 0, k = 0;
        int iBranch = 0;
        int curBranchID = mySolNode.branchID[iBranch];
        int sumOrbit = mySolNode.numOrbitsInEachBranch[iBranch];

        for(long int ka=0; ka<mySolNode.numOrbits; ka++)
        {
            if(ka >= sumOrbit)
            {
                curBranchID = mySolNode.branchID[++iBranch];
                sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
            }

            k = k+1;
            solGroup->addChild(drawAnOrbitUsingLines(iBranch, k, si, lineWidthScaler,\
                clientData.labelIndex[k][3], clientData.labelIndex[k][2], true));
            si += mySolNode.numVerticesEachPeriod[ka];
        }
    }
    else if(animationLabel != MY_NONE)
    {
        for(int n=0; n<lblIdxSize; ++n)
        {
            animationLabel=myLabels[lblIndices[n]];
            int si = 0, k = 0;
            int iBranch = 0;
            int curBranchID = mySolNode.branchID[iBranch];
            int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
            for(int ka=0; ka<mySolNode.numOrbits; ka++)
            {
                if(ka >= sumOrbit)
                {
                    curBranchID = mySolNode.branchID[++iBranch];
                    sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
                }

                k = k+1;
                if(myLabels[ka+1]>=animationLabel) break;
                si += mySolNode.numVerticesEachPeriod[ka];
            }

            if(options[OPT_SAT_ANI])
            {
                solGroup->addChild(animateOrbitWithTail(iBranch, k, si));
            }
            else
            {
                solGroup->addChild(drawAnOrbitUsingLines(iBranch, k, si, lineWidthScaler,\
                    clientData.labelIndex[k][3], clientData.labelIndex[k][2], true));
            }
        }
    }

    if(options[OPT_PERIOD_ANI])
    {
        bool coloring;
        coloring = (animationLabel == 0) ? false : true;
        solGroup->addChild(animateSolutionUsingLines(coloring));
    }

    return solGroup;
}


///////////////////////////////////////////////////////////////////////
//
//           draw the solutions using NURBS CURVE lines
//
SoGroup *
renderSolutionNurbsCurve()
//
//////////////////////////////////////////////////////////////////////////
{
    SoGroup * solGroup = new SoGroup;

    SoCoordinate3 *solCoords = new SoCoordinate3;

    if(animationLabel == 0)
    {
        long int si = 0, k = 0;
        int iBranch = 0;
        int curBranchID = mySolNode.branchID[iBranch];
        int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
        for(long int ka=0; ka<mySolNode.numOrbits; ka++)
        {
            if(ka >= sumOrbit)
            {
                curBranchID = mySolNode.branchID[++iBranch];
                sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
            }
            k = k+1;
            solGroup->addChild(drawAnOrbitUsingNurbsCurve(iBranch, k, si, lineWidthScaler,\
                clientData.labelIndex[k][3], clientData.labelIndex[k][2]));
            si += mySolNode.numVerticesEachPeriod[ka];
        }
    }
    else if(animationLabel != MY_NONE)
    {
        for(int n=0; n<lblIdxSize; ++n)
        {
            animationLabel=myLabels[lblIndices[n]];
            int si = 0, k = 0;
            int iBranch = 0;
            int curBranchID = mySolNode.branchID[iBranch];
            int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
            for(int ka=0; ka<mySolNode.numOrbits; ka++)
            {
                if(ka >= sumOrbit)
                {
                    curBranchID = mySolNode.branchID[++iBranch];
                    sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
                }
                k = k+1;
                if(myLabels[ka+1]>=animationLabel) break;
                si += mySolNode.numVerticesEachPeriod[ka];
            }

            if(options[OPT_SAT_ANI])
            {
                solGroup->addChild(animateOrbitWithTail(iBranch, k, si));
            }
            else
            {
                solGroup->addChild(drawAnOrbitUsingNurbsCurve(iBranch, k, si, lineWidthScaler,\
                    clientData.labelIndex[k][3], clientData.labelIndex[k][2]));
            }
        }
    }

    if(options[OPT_PERIOD_ANI])
    {
        bool coloring = (animationLabel == 0) ? true : false;
        solGroup->addChild(animateSolutionUsingLines(coloring));
    }
    return solGroup;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
animateSolutionUsingPoints(int style, bool aniColoring)
//
//////////////////////////////////////////////////////////////////////////
{
    SoSeparator *solGroup = new SoSeparator;
    SoSeparator *solStand = new SoSeparator;
    int si = 0;
    SoBlinker * solBlinker = new SoBlinker;
    solBlinker->speed = orbitSpeed;
    solBlinker->on = TRUE;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    int sumOrbit = mySolNode.numOrbitsInEachBranch[iBranch];
    for(int l=0; l<mySolNode.numOrbits; l++)
    {
        long numVertices = mySolNode.numVerticesEachPeriod[l];
        if(l >= sumOrbit)
        {
            curBranchID = mySolNode.branchID[++iBranch];
            sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
        }

        if(numVertices == 1)
            solStand->addChild(drawAnOrbitUsingPoints(style, iBranch,/* curBranchID,*/ l+1, si, aniLineScaler*lineWidthScaler,\
                clientData.labelIndex[l+1][3], clientData.labelIndex[l+1][2], aniColoring));
        else
            solBlinker->addChild(drawAnOrbitUsingPoints(style, iBranch,/* curBranchID,*/ l+1, si, aniLineScaler*lineWidthScaler,\
                clientData.labelIndex[l+1][3], clientData.labelIndex[l+1][2], aniColoring));
        si+=mySolNode.numVerticesEachPeriod[l];
    }
    solGroup->addChild(solStand);
    solGroup->addChild(solBlinker);
    return solGroup;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
animateSolutionUsingLines(bool aniColoring)
//
//////////////////////////////////////////////////////////////////////////
{
// the following is the line version
    SoSeparator *solGroup = new SoSeparator;
    SoSeparator *solStand = new SoSeparator;
    int si = 0;
    SoBlinker * solBlinker = new SoBlinker;
    solBlinker->speed = orbitSpeed;
    solBlinker->on = TRUE;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    int sumOrbit = mySolNode.numOrbitsInEachBranch[iBranch];
    for(int l=0; l<mySolNode.numOrbits; l++)
    {
        long numVertices = mySolNode.numVerticesEachPeriod[l];
        if(l >= sumOrbit)
        {
            curBranchID = mySolNode.branchID[++iBranch];
            sumOrbit+= mySolNode.numOrbitsInEachBranch[iBranch];
        }

        if(numVertices == 1)
            solStand->addChild(drawAnOrbitUsingLines(iBranch, l+1, si, aniLineScaler*lineWidthScaler,\
                clientData.labelIndex[l+1][3], clientData.labelIndex[l+1][2], aniColoring));
        else
            solBlinker->addChild(drawAnOrbitUsingLines(iBranch, l+1, si, aniLineScaler*lineWidthScaler,\
                clientData.labelIndex[l+1][3], clientData.labelIndex[l+1][2], aniColoring));
        si+=mySolNode.numVerticesEachPeriod[l];
    }
    solGroup->addChild(solStand);
    solGroup->addChild(solBlinker);
    return solGroup;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
animateSolutionUsingNurbsCurve()
//
/////////////////////////////////////////////////////////////////
{
    SoSeparator *solGroup = new SoSeparator;
    long int si = 0;
    SoBlinker * solBlinker = new SoBlinker;
    solBlinker->speed = orbitSpeed;
    solBlinker->on = TRUE;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
    for(long int l=0; l<mySolNode.numOrbits; l++)
    {
        if(l >= sumOrbit)
        {
            curBranchID = mySolNode.branchID[++iBranch];
            sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
        }
        solBlinker->addChild(drawAnOrbitUsingNurbsCurve(iBranch, l+1, si, lineWidthScaler,\
            clientData.labelIndex[l+1][3], clientData.labelIndex[l+1][2]));
        si+=mySolNode.numVerticesEachPeriod[l];
    }
    solGroup->addChild(solBlinker);
    return solGroup;
}


/////////////////////////////////////////////////////////////////
//                  create solution orbits scene
SoSeparator *
renderSolution()
//
/////////////////////////////////////////////////////////////////
{
    SoSeparator *solSep = new SoSeparator;
    SoGroup *solGroup = new SoGroup;

    SoCoordinate3 *solCoords = new SoCoordinate3;
    if(whichStyle==TUBE)
    {
        solGroup->addChild(renderSolutionTubes());
    }
    else if(whichStyle==SURFACE)
    {
        solGroup->addChild(renderSolutionSurface());
    }
    else if(whichStyle==NURBS) 
    {
        solGroup->addChild(renderSolutionNurbsCurve());
    }
    else if(whichStyle==ALL_POINTS || whichStyle == MESH_POINTS)
    {
        solGroup->addChild(renderSolutionPoints(whichStyle));
    }
    else 
    {
        solGroup->addChild(renderSolutionLines());
    }
    solSep->addChild(solGroup);
    return solSep;
}


//////////////////////////////// START \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
//
// draw a strip by giving the stripset
SoSeparator *
drawAStrip(float stripSet[][3], int size)
//
//////////////////////////////////////////////////////////////////////
{
    SoSeparator * myStrip = new SoSeparator;
    SoShapeHints * solHints = new SoShapeHints;
    solHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
    myStrip->addChild(solHints);

    SoCoordinate3 *solCoords = new SoCoordinate3;
    solCoords->point.setValues(0, size, stripSet);

    int32_t mySize[1];
    mySize[0] = size;
    SoTriangleStripSet *solStrips = new SoTriangleStripSet;
    solStrips->numVertices.setValues(0, 1, mySize);

    myStrip->addChild(solCoords);
    myStrip->addChild(solStrips);
    return myStrip;
}


//////////////////////////////////////////////////////////////////////////
//
SoGroup *
setLineAttributesByParameterValue(double parValue, double parMax, 
           double parMid, double parMin, int stability, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    legendScaleValues[0] = parMin;
    legendScaleValues[1] = parMax;

    SoDrawStyle * lineStyle = new SoDrawStyle;
    if(stability == 1 || stability == 3)
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[0];
    }
    else
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[1];
    }

    SoMaterial * lineMtl = new SoMaterial;
    lineMtl->diffuseColor.setValue(1.0, 1.0, 1.0);
    lineMtl->transparency = 0.0;

    double colFactor;
    if(parMax != parMin)
    {
        if(parValue > parMid)
        {
            colFactor = (parValue-parMid)/(parMax-parMid);
            lineMtl->diffuseColor.setValue(1.0*colFactor, 1.0*(1-colFactor), 0.0);
        }
        else
        {
            colFactor = (parValue-parMin)/(parMid-parMin);
            lineMtl->diffuseColor.setValue(0.0*colFactor, 1.0*colFactor, 1.0*(1-colFactor));
        }
    }

    lineStyle->lineWidth   = 1.5*scaler;

    lineStyle->lineWidth = scaler;

    SoGroup * lineAttributes = new SoGroup;
    lineAttributes->addChild(lineStyle);
    lineAttributes->addChild(lineMtl);

    return lineAttributes;
}


//////////////////////////////////////////////////////////////////////////
//
//  Set the line pattern and color for the orbit according to
//  its stablity and type.
//
SoGroup *
setLineAttributesByBranch(int branchID, int stability, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    SoDrawStyle * lineStyle = new SoDrawStyle;
    lineStyle->style = SoDrawStyle::FILLED;
    if(stability == 1 || stability == 3)
        lineStyle->linePattern = stabilityLinePattern[0];
    else
        lineStyle->linePattern = stabilityLinePattern[1];

    SoMaterial * lineMtl = new SoMaterial;
    lineMtl->diffuseColor.setValue(1.0, 1.0, 1.0);
    lineMtl->transparency = 0.0;

    lineStyle->lineWidth   = 1.5*scaler;
    switch(abs(branchID)%13)
    {
        case 0:
            lineMtl->diffuseColor.setValue(lineColor[0]);
            lineStyle->lineWidth = scaler;
            break;
        case 1:
            lineMtl->diffuseColor.setValue(lineColor[1]);
            break;
        case 2:
            lineMtl->diffuseColor.setValue(lineColor[2]);
            break;
        case 3:
            lineMtl->diffuseColor.setValue(lineColor[3]);
            break;
        case 4:
            lineMtl->diffuseColor.setValue(lineColor[4]);
            break;
        case 5:
            lineMtl->diffuseColor.setValue(lineColor[5]);
            break;
        case 6:
            lineMtl->diffuseColor.setValue(lineColor[6]);
            break;
        case 7:
            lineMtl->diffuseColor.setValue(lineColor[7]);
            break;
        case 8:
            lineMtl->diffuseColor.setValue(lineColor[8]);
            break;
        case 9:
            lineMtl->diffuseColor.setValue(lineColor[9]);
            break;
        case 10:
            lineMtl->diffuseColor.setValue(lineColor[10]);
            lineStyle->lineWidth = scaler;
            break;
        case 11:
            lineMtl->diffuseColor.setValue(lineColor[11]);
            lineStyle->lineWidth = scaler;
            break;
        case 12:
        default:
            lineMtl->diffuseColor.setValue(lineColor[12]);
            lineStyle->lineWidth = scaler;
            break;
    }

    SoGroup * lineAttributes = new SoGroup;
    lineAttributes->addChild(lineStyle);
    lineAttributes->addChild(lineMtl);
    return lineAttributes;
}


//////////////////////////////////////////////////////////////////////////
//
SoGroup *
setLineColorBlendingByStability(float * vertices, long int size, int stability, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    float (*colors)[3] = new float[size][3];
    static float maxColor[3];
    static float minColor[3]; 

    for(int i=0; i<3; ++i)
    {
        maxColor[i] = envColors[6][i];
        minColor[i] = envColors[7][i];
    }

    for(int i=0; i<size; ++i)
    {
        if(vertices[i]== 1 || vertices[i]== 3)
            for(int j=0; j<3; ++j) colors[i][j] = maxColor[j];
        else
            for(int j=0; j<3; ++j) colors[i][j] = minColor[j];
    }

    SoGroup * result = new SoGroup ;

    SoDrawStyle * lineStyle = new SoDrawStyle;
    if(stability == 1 || stability == 3)
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[0];
    }
    else
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[1];
    }
    lineStyle->lineWidth = scaler;
    lineStyle->lineWidth = scaler;
    result->addChild(lineStyle);

    SoMaterial *myMaterials = new SoMaterial;
    myMaterials->diffuseColor.setValues(0, size, colors);
    result->addChild(myMaterials);

    SoMaterialBinding *myMaterialBinding = new SoMaterialBinding;
    myMaterialBinding->value = SoMaterialBinding::PER_VERTEX;
    result->addChild(myMaterialBinding);
    delete [] colors;
    return result;
}


//////////////////////////////////////////////////////////////////////////
//
SoGroup *
setLineAttributesByStability(int stability, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    SoDrawStyle * lineStyle = new SoDrawStyle;
    lineStyle->style = SoDrawStyle::FILLED;

    SoMaterial * lineMtl = new SoMaterial;
    lineMtl->shininess = 0.9;

    if(stability == 1 || stability == 3)
    {
        lineStyle->linePattern = stabilityLinePattern[0];
        lineMtl->diffuseColor.setValue(envColors[6]);
    }
    else
    {
        lineStyle->linePattern = stabilityLinePattern[1];
        lineMtl->diffuseColor.setValue(envColors[7]);
    }

    lineStyle->lineWidth   = scaler;

    SoGroup * lineAttributes = new SoGroup;
    lineAttributes->addChild(lineStyle);
    lineAttributes->addChild(lineMtl);
    return lineAttributes;
}


//////////////////////////////////////////////////////////////////////////
//
//  Set the line pattern and color for the orbit according to
//  its stablity and type.
//
SoGroup *
setLineAttributesByType(int stability, int type, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    SoDrawStyle * lineStyle = new SoDrawStyle;
    if(stability == 1 || stability == 3)
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[0];
    }
    else
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[1];
    }

    SoMaterial * lineMtl = new SoMaterial;
    lineMtl->shininess = 0.9;

    lineStyle->lineWidth   = 1.5*scaler;
    switch(type)
    {
        case 0:
            lineMtl->diffuseColor.setValue(lineColor[0]);
            lineStyle->lineWidth = scaler;
            break;
        case 1:
            lineMtl->diffuseColor.setValue(lineColor[1]);
            break;
        case 2: 
            lineMtl->diffuseColor.setValue(lineColor[2]);
            break;
        case 3: 
            lineMtl->diffuseColor.setValue(lineColor[3]);
            break;
        case 4: 
            lineMtl->diffuseColor.setValue(lineColor[4]);
            break;
        case -4:
            lineMtl->diffuseColor.setValue(lineColor[5]);
            break;
        case 5:
            lineMtl->diffuseColor.setValue(lineColor[6]);
            break;
        case 6: 
            lineMtl->diffuseColor.setValue(lineColor[7]);
            break;
        case 7:
            lineMtl->diffuseColor.setValue(lineColor[8]);
            break;
        case 8:
            lineMtl->diffuseColor.setValue(lineColor[9]);
            break;
        case 9:
            lineMtl->diffuseColor.setValue(lineColor[10]);
            lineStyle->lineWidth = scaler;
            break;
        case -9:
            lineMtl->diffuseColor.setValue(lineColor[11]);
            lineStyle->lineWidth = scaler;
            break;
        default:
            lineMtl->diffuseColor.setValue(lineColor[12]);
            lineStyle->lineWidth = scaler;
            break;
    }

    SoGroup * lineAttributes = new SoGroup;
    lineAttributes->addChild(lineStyle);
    lineAttributes->addChild(lineMtl);
    return lineAttributes;
}


//////////////////////////////////////////////////////////////////////////
//
SoGroup *
setLineColorBlending(float * vertices, long int size, int stability, int type, float scaler)
//
//////////////////////////////////////////////////////////////////////////
{
    float (*colors)[3] = new float[size][3];
    static float maxColor[3] =                    //red
    {
        1.0, 0.0, 0.0
    };
    static float midColor[3] =                    //green
    {
        0.0, 1.0, 0.0
    };
    static float minColor[3] =                    //blue
    {
        0.0, 0.0, 1.0
    };

    float maxValue = vertices[0];
    float minValue = vertices[0];
    float midValue = vertices[0];

    for(int i=0; i<size; ++i)
    {
        if(maxValue<vertices[i])maxValue=vertices[i];
        if(minValue>vertices[i])minValue=vertices[i];
    }

    if( whichType == SOLUTION &&
        (coloringMethod != CL_POINT_NUMBER && animationLabel == 0 || options[OPT_PERIOD_ANI] ))
    {
        legendScaleValues[0]= minValue = clientData.solMin[coloringMethod];
        legendScaleValues[1]= maxValue = clientData.solMax[coloringMethod];
    }
    else
    {
        legendScaleValues[0]= minValue;
        legendScaleValues[1]= maxValue;
    }
    midValue = (maxValue + minValue)/2.0;

    float dt = maxValue - minValue;
    float dtColor;
    for(int i=0; i<size; ++i)
    {
        for(int j=0; j<3; ++j)
        {
            if(vertices[i]>midValue)
            {
                dt = maxValue - midValue;
                dtColor = maxColor[j]-midColor[j];
                colors[i][j] = (dt<1.0e-9) ?
                    ((midColor[j]+maxColor[j])/2.0) :
                (midColor[j] + dtColor * (vertices[i]-midValue)/dt);
            }
            else
            {
                dt = midValue - minValue;
                dtColor = midColor[j]-minColor[j];
                colors[i][j] = (dt<1.0e-9) ?
                    ((minColor[j]+midColor[j])/2.0) :
                (minColor[j] + dtColor * (vertices[i]-minValue)/dt);
            }
        }
    }

    SoGroup * result = new SoGroup ;

// set line style and pattern
    SoDrawStyle * lineStyle = new SoDrawStyle;
    if(stability == 1 || stability == 3)
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[0];
    }
    else
    {
        lineStyle->style = SoDrawStyle::FILLED;
        lineStyle->linePattern = stabilityLinePattern[1];
    }
    lineStyle->lineWidth = scaler;
    result->addChild(lineStyle);

    SoMaterial *myMaterials = new SoMaterial;
    myMaterials->diffuseColor.setValues(0, size, colors);
    result->addChild(myMaterials);

    SoMaterialBinding *myMaterialBinding = new SoMaterialBinding;
    myMaterialBinding->value = SoMaterialBinding::PER_VERTEX;
    result->addChild(myMaterialBinding);
    delete [] colors;
    return result;
}


///////////////////////////////////////////////////////////////////////////
//
//
SoSeparator *
drawAnOrbitUsingLines(int iBranch,  long int l, long int si, 
       float scaler, int stability, int type, bool aniColoring)
//
//////////////////////////////////////////////////////////////////////////
{
    SoSeparator * anOrbit = new SoSeparator;
    int32_t  myint[10];

    float dis = !options[OPT_NORMALIZE_DATA] ? 
	               (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0;

    long numVertices = mySolNode.numVerticesEachPeriod[l-1];
    if(numVertices == 1 )
    {
        long int idx = si;
        SoSeparator * ptSep = new SoSeparator;

        if(coloringMethod == CL_BRANCH_NUMBER)
            ptSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
        else if(coloringMethod == CL_STABILITY)
            ptSep->addChild(setLineAttributesByStability(stability, scaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            ptSep->addChild(setLineAttributesByType(stability, type, scaler));
        else if(coloringMethod == CL_LABELS)
        {
            ptSep->addChild(setLineAttributesByParameterValue(
                l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                stability, scaler));
        }
        else if(coloringMethod == CL_COMPONENT)
        {
             ptSep->addChild(setLineAttributesByParameterValue(
                 curComponent, maxComponent, maxComponent/2.0, 0,
                 stability, scaler));
        }
        else 
        {
            SoMaterial * ptMtl = new SoMaterial;
            ptMtl->diffuseColor.setValue(1,0,0);
            ptSep->addChild(ptMtl);
        }

        
        float ver[2][3];

        if(time_on != TIME_IS_OFF)
        {
            if(time_on == TIME_ON_X)
            {
                ver[0][0] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[0]-mySolNode.min[0]) : 0;
                ver[0][1] = mySolNode.xyzCoords[idx][1];
                ver[0][2] = 0;
                ver[1][0] = (mySolNode.max[0] <= 1.0) ? 1.0 : mySolNode.max[0];
                ver[1][1] = mySolNode.xyzCoords[idx][1];
                ver[1][2] = 0;
            }
            else if(time_on == TIME_ON_Y)
            {
                ver[0][0] = mySolNode.xyzCoords[idx][0];
                ver[0][1] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[1]-mySolNode.min[1]) : 0;
                ver[0][2] = 0;
                ver[1][0] = mySolNode.xyzCoords[idx][0]; 
                ver[1][1] = ( mySolNode.max[1] <= 1.0 ) ? 1.0 : mySolNode.max[1];
                ver[1][2] = 0;
            }
            else if(time_on == TIME_ON_Z)
            {
                ver[0][0] = mySolNode.xyzCoords[idx][0];
                ver[0][1] = 0; 
                ver[0][2] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[2]-mySolNode.min[2]) : 0; 
                ver[1][0] = mySolNode.xyzCoords[idx][0];
                ver[1][1] = 0; 
                ver[1][2] = ( mySolNode.max[2] <= 1.0 ) ? 1.0 : mySolNode.max[2]; 
            }

            SoCoordinate3 *myC = new SoCoordinate3;
            myC->point.setValues(0, 2, ver);
            myint[0]=2;
            SoLineSet *myL = new SoLineSet;
            myL->numVertices.setValues(0, 1, myint);
            ptSep->addChild(myC);
            ptSep->addChild(myL);
            anOrbit->addChild(ptSep);
        }
        else
        {
            SoTransform * aTrans = new SoTransform;
            aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
		           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
            ptSep->addChild(aTrans);
            SoSphere *aPoint = new SoSphere;
            aPoint->radius = dis * STATIONARY_POINT_RADIUS;
            ptSep->addChild(aPoint);
            anOrbit->addChild(ptSep);
        }
        return anOrbit;
    }

    float (*vertices)[3] = new float[numVertices][3];
    float *colorBase = new float[numVertices];

    for(int m=0; m<numVertices; m++)
    {
        long int idx = si+m;
        vertices[m][0]=mySolNode.xyzCoords[idx][0];
        vertices[m][1]=mySolNode.xyzCoords[idx][1];
        vertices[m][2]=mySolNode.xyzCoords[idx][2];
        if(coloringMethod>=0)colorBase[m]  = clientData.solData[idx][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)colorBase[m]  = m;
    }

    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, mySolNode.numVerticesEachPeriod[l-1], vertices);
    myint[0]=mySolNode.numVerticesEachPeriod[l-1];

    SoLineSet *myLine= new SoLineSet;
    myLine->numVertices.setValues(0,1,myint);
//    if(maxComponent == 1)
//    {
        if(!aniColoring)
        {
            anOrbit->addChild(setLineAttributesByType(stability, 0, scaler));
        }
        else
        {
            if(coloringMethod == CL_BRANCH_NUMBER)
                anOrbit->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
            else if(coloringMethod == CL_STABILITY)
                anOrbit->addChild(setLineAttributesByStability(stability, scaler));
            else if(coloringMethod == CL_ORBIT_TYPE)
                anOrbit->addChild(setLineAttributesByType(stability, type, scaler));
            else if(coloringMethod == CL_LABELS)
            {
/*
                double bMin = 0;
                for(int ib = 0; ib< iBranch; ++ib)
                    bMin +=  mySolNode.numOrbitsInEachBranch[ib];
                double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
                anOrbit->addChild(setLineAttributesByParameterValue(
                    l-1, bMax, (bMax+bMin)/2.0, bMin,
                    stability, scaler));
*/
                anOrbit->addChild(setLineAttributesByParameterValue(
                     l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                     stability, scaler));
            }
            else if(coloringMethod == CL_COMPONENT)
                anOrbit->addChild(setLineAttributesByParameterValue(
                        curComponent, maxComponent, maxComponent/2.0, 0,
                        stability, scaler));
            else if(coloringMethod >= mySolNode.nar)
                anOrbit->addChild(setLineAttributesByParameterValue(
                    mySolNode.par[l-1][mySolNode.parID[coloringMethod-mySolNode.nar]],
                    mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                    mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                    mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                    stability, scaler));
            else
                anOrbit->addChild(setLineColorBlending(colorBase,
                    mySolNode.numVerticesEachPeriod[l-1],stability, type, scaler));
        }
//    }else
//    anOrbit->addChild(setLineAttributesByParameterValue(
//           curComponent, maxComponent, maxComponent/2.0, 0,
//          stability, scaler));

    anOrbit->addChild(myCoords);
    anOrbit->addChild(myLine);

    delete [] vertices;
    delete [] colorBase;

    return anOrbit;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawAPoint(float x, float y, float z, float size, float scale)
//
//////////////////////////////////////////////////////////////////////////
{
    SoSeparator * ptSep = new SoSeparator;
    SoTransform * ptTrans = new SoTransform;
    ptTrans->translation.setValue(x, y, z);
    ptSep->addChild(ptTrans);

    SoCube *aPoint = new SoCube;
    aPoint->width = size*scale;
    aPoint->height= size*scale;
    aPoint->depth = size*scale;
    ptSep->addChild(aPoint);
    return ptSep;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawAnOrbitUsingPoints(int style, int iBranch,  long int l, 
     long int si, float scaler, int stability, int type, bool aniColoring)
//
//////////////////////////////////////////////////////////////////////////
{
    SoSeparator * anOrbit = new SoSeparator;

    float dis = !options[OPT_NORMALIZE_DATA] ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0;

    long numVertices = mySolNode.numVerticesEachPeriod[l-1];
    if(numVertices == 1)
    {
        long int idx = si;
        SoSeparator * ptSep = new SoSeparator;
        SoTransform * aTrans = new SoTransform;

        if(coloringMethod == CL_BRANCH_NUMBER)
            ptSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
        else if(coloringMethod == CL_STABILITY)
            ptSep->addChild(setLineAttributesByStability(stability, scaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            ptSep->addChild(setLineAttributesByType(stability, type, scaler));
        else if(coloringMethod == CL_LABELS)
        {
            ptSep->addChild(setLineAttributesByParameterValue(
                l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                stability, scaler));
        }
        else if(coloringMethod == CL_COMPONENT)
        {
             ptSep->addChild(setLineAttributesByParameterValue(
                 curComponent, maxComponent, maxComponent/2.0, 0,
                 stability, scaler));
        }
        else 
        {
            SoMaterial * ptMtl = new SoMaterial;
            ptMtl->diffuseColor.setValue(1,0,0);
            ptSep->addChild(ptMtl);
        }

        aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
		           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
        ptSep->addChild(aTrans);

        SoSphere *aPoint = new SoSphere;
        aPoint->radius = dis * STATIONARY_POINT_RADIUS;
        ptSep->addChild(aPoint);
        anOrbit->addChild(ptSep);
        return anOrbit;
    }

    int32_t  myint[10];
    float (*vertices)[3] = new float[numVertices][3];
    float *colorBase = new float[numVertices];

    for(int m=0; m<numVertices; m++)
    {
        long int idx = si+m;
        vertices[m][0]=mySolNode.xyzCoords[idx][0];
        vertices[m][1]=mySolNode.xyzCoords[idx][1];
        vertices[m][2]=mySolNode.xyzCoords[idx][2];
        if(coloringMethod>=0)colorBase[m]  = clientData.solData[idx][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)colorBase[m]  = m;

//        if(maxComponent == 1)
//        {
            if(!aniColoring)
            {
                anOrbit->addChild(setLineAttributesByType(stability, 0, scaler));
            }
            else
            {
                if(coloringMethod == CL_BRANCH_NUMBER)
                    anOrbit->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
                else if(coloringMethod == CL_STABILITY)
                    anOrbit->addChild(setLineAttributesByStability(stability, scaler));
                else if(coloringMethod == CL_ORBIT_TYPE)
                    anOrbit->addChild(setLineAttributesByType(stability, type, scaler));
                else if(coloringMethod == CL_LABELS)
                {
/*
                    double bMin = 0;
                    for(int ib = 0; ib< iBranch; ++ib)
                        bMin +=  mySolNode.numOrbitsInEachBranch[ib];
                    double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
                    anOrbit->addChild(setLineAttributesByParameterValue(
                        l-1, bMax, (bMax+bMin)/2.0, bMin, stability, scaler));
*/
                    anOrbit->addChild(setLineAttributesByParameterValue(
                         l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                         stability, scaler));
                }
                else if(coloringMethod == CL_COMPONENT)
                    anOrbit->addChild(setLineAttributesByParameterValue(
                        curComponent, maxComponent, maxComponent/2.0, 0,
                        stability, scaler));
                else if(coloringMethod >= mySolNode.nar)
                {
                    anOrbit->addChild(setLineAttributesByParameterValue(
                        mySolNode.par[l-1][mySolNode.parID[coloringMethod-mySolNode.nar]],
                        mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                        mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                        mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                        stability, scaler));
                }
                else
                    anOrbit->addChild(setLineColorBlending(colorBase,
                        mySolNode.numVerticesEachPeriod[l-1],stability, type, scaler));
            }
//        }else
//        anOrbit->addChild(setLineAttributesByParameterValue(
//                curComponent, maxComponent, maxComponent/2.0, 0,
//                stability, scaler));

        if(style == MESH_POINTS)
        {
            if(m%mySolNode.ncol[l-1] == 0)
                anOrbit->addChild(drawAPoint(mySolNode.xyzCoords[idx][0], mySolNode.xyzCoords[idx][1],
                    mySolNode.xyzCoords[idx][2], dis, STATIONARY_POINT_RADIUS*0.5));
        }else
        anOrbit->addChild(drawAPoint(mySolNode.xyzCoords[idx][0], mySolNode.xyzCoords[idx][1],
                mySolNode.xyzCoords[idx][2], dis, STATIONARY_POINT_RADIUS*0.5));
    }


    delete [] vertices;
    delete [] colorBase;

    return anOrbit;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawAnOrbitUsingNurbsCurve(int iBranch, long int l, long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    int32_t  myint[10];
    SoSeparator * anOrbit = new SoSeparator;
    float (*vertices)[3];
    vertices = new float[mySolNode.numVerticesEachPeriod[l-1]][3];
    for(int m=0; m<mySolNode.numVerticesEachPeriod[l-1]; m++)
    {
        vertices[m][0]=mySolNode.xyzCoords[si+m][0];
        vertices[m][1]=mySolNode.xyzCoords[si+m][1];
        vertices[m][2]=mySolNode.xyzCoords[si+m][2];
    }
    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, mySolNode.numVerticesEachPeriod[l-1], vertices);
    myint[0]=mySolNode.numVerticesEachPeriod[l-1];

    int number = mySolNode.numVerticesEachPeriod[l-1];
    float * knots = new float[number+4];
    for (int i=0; i<4; ++i) knots[i]=0, knots[i+number]=number-3;
    for(int i=4; i<number; ++i) knots[i]=i-3;
    SoNurbsCurve *myCurve = new SoNurbsCurve;
    myCurve->numControlPoints = mySolNode.numVerticesEachPeriod[l-1];
    myCurve->knotVector.setValues(0, number+4, knots);

    if(coloringMethod == CL_BRANCH_NUMBER)
        anOrbit->addChild(setLineAttributesByBranch(iBranch,stability,scaler));
    else if(coloringMethod == CL_STABILITY)
        anOrbit->addChild(setLineAttributesByStability(stability, scaler));
    else if(coloringMethod == CL_LABELS)
    {
        anOrbit->addChild(setLineAttributesByParameterValue(
           l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
           stability, scaler));
    }
    else
        anOrbit->addChild(setLineAttributesByType(stability, type, scaler));
    anOrbit->addChild(myCoords);
    anOrbit->addChild(myCurve);
    delete [] vertices;
    delete [] knots;
    return anOrbit;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawAnOrbitUsingTubes(int iBranch, long int l, long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    float dis = !options[OPT_NORMALIZE_DATA] ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0 ;
    int32_t  myint[10];

    SoSeparator * anOrbit = new SoSeparator;
    long int sumX = 0;
    long int numVertices = mySolNode.numVerticesEachPeriod[l-1];
    if(numVertices == 1)
    {
        int idx = si;
        SoSeparator * ptSep = new SoSeparator;
        SoSphere * aPoint = new SoSphere;
        aPoint->radius = dis * STATIONARY_POINT_RADIUS;

        if(coloringMethod == CL_BRANCH_NUMBER)
            ptSep->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
        else if(coloringMethod == CL_STABILITY)
            ptSep->addChild(setLineAttributesByStability(stability, scaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            ptSep->addChild(setLineAttributesByType(stability, type, scaler));
        else if(coloringMethod == CL_LABELS)
        {
            anOrbit->addChild(setLineAttributesByParameterValue(
                l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                stability, scaler));
        }
        else if(coloringMethod == CL_COMPONENT)
        {
             anOrbit->addChild(setLineAttributesByParameterValue(
                 curComponent, maxComponent, maxComponent/2.0, 0,
                 stability, scaler));
        }
        else 
        {
            SoMaterial * ptMtl = new SoMaterial;
            ptMtl->diffuseColor.setValue(1,0,0);
            ptSep->addChild(ptMtl);
        }

        float ver[2][3];

        if(time_on != TIME_IS_OFF)
        {
            if(time_on == TIME_ON_X)
            {
                ver[0][0] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0 ; //-(mySolNode.max[0]-mySolNode.min[0]) : 0;
                ver[0][1] = mySolNode.xyzCoords[idx][1];
                ver[0][2] = 0;
                ver[1][0] = (mySolNode.max[0] <= 1.0) ? 1.0 : mySolNode.max[0];
                ver[1][1] = mySolNode.xyzCoords[idx][1];
                ver[1][2] = 0;
            }
            else if(time_on == TIME_ON_Y)
            {
                ver[0][0] = mySolNode.xyzCoords[idx][0];
                ver[0][1] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[1]-mySolNode.min[1]) : 0;
                ver[0][2] = 0;
                ver[1][0] = mySolNode.xyzCoords[idx][0]; 
                ver[1][1] = ( mySolNode.max[1] <= 1.0 ) ? 1.0 : mySolNode.max[1];
                ver[1][2] = 0;
            }
            else if(time_on == TIME_ON_Z)
            {
                ver[0][0] = mySolNode.xyzCoords[idx][0];
                ver[0][1] = 0; 
                ver[0][2] = (options[OPT_NORMALIZE_DATA]) ? -1 : 0; //-(mySolNode.max[2]-mySolNode.min[2]) : 0; 
                ver[1][0] = mySolNode.xyzCoords[idx][0];
                ver[1][1] = 0; 
                ver[1][2] = ( mySolNode.max[2] <= 1.0 ) ? 1.0 : mySolNode.max[2]; 
            }
            SoCoordinate3 *myC = new SoCoordinate3;
            myC->point.setValues(0, 2, ver);
            myint[0]=2;
            SoLineSet *myL = new SoLineSet;
            myL->numVertices.setValues(0, 1, myint);
            ptSep->addChild(myC);
            ptSep->addChild(myL);
            anOrbit->addChild(ptSep);
        }
        else
        {
            SoTransform * aTrans = new SoTransform;
            aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
		           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
            ptSep->addChild(aTrans);
            SoSphere *aPoint = new SoSphere;
            aPoint->radius = dis * STATIONARY_POINT_RADIUS;
            ptSep->addChild(aPoint);
            anOrbit->addChild(ptSep);
        }
/*
        SoTransform * aTrans = new SoTransform;
        aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
		           mySolNode.xyzCoords[idx][1], mySolNode.xyzCoords[idx][2]);
        ptSep->addChild(aTrans);
        ptSep->addChild(aPoint);
        anOrbit->addChild(ptSep);
*/
        return anOrbit;
    }
    else if( numVertices < 1 )
    {
        return anOrbit;
    }

    float (*vertices)[3] = new float[mySolNode.numVerticesEachPeriod[l-1]][3];
    float *colorBase = new float[mySolNode.numVerticesEachPeriod[l-1]*11];
    Tube tube;
    for(int m=0; m<mySolNode.numVerticesEachPeriod[l-1]; m++)
    {
        vertices[m][0]=mySolNode.xyzCoords[si+m][0];
        vertices[m][1]=mySolNode.xyzCoords[si+m][1];
        vertices[m][2]=mySolNode.xyzCoords[si+m][2];
        if(coloringMethod>=0)
            for(int j=0; j<11; ++j)
                colorBase[m*11+j]  = clientData.solData[si+m][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)
            for(int j=0; j<11; ++j)
                colorBase[m*11+j]  = m;
    }
    tube = Tube(mySolNode.numVerticesEachPeriod[l-1], vertices, lineWidthScaler*0.005, 10);

//    if(maxComponent == 1)
//    {
        if(coloringMethod == CL_BRANCH_NUMBER)
            anOrbit->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
        else if(coloringMethod == CL_STABILITY)
            anOrbit->addChild(setLineAttributesByStability(stability, scaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            anOrbit->addChild(setLineAttributesByType(stability, type, scaler));
        else if(coloringMethod == CL_LABELS)
        {
/*
//          always start from blue to red for different branches. 
            double bMin = 0;
            for(int ib = 0; ib< iBranch; ++ib)
                bMin +=  mySolNode.numOrbitsInEachBranch[ib];
            double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
            anOrbit->addChild(setLineAttributesByParameterValue(
                l-1, bMax, (bMax+bMin)/2.0, bMin,
                stability, scaler));
*/
//          always set the first label blue, the last red, namely look all
//          branches as one.
            anOrbit->addChild(setLineAttributesByParameterValue(
                l-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                stability, scaler));
        }
        else if(coloringMethod == CL_COMPONENT)
        {
            anOrbit->addChild(setLineAttributesByParameterValue(
                curComponent, maxComponent, maxComponent/2.0, 0,
                stability, scaler));
        }
        else if(coloringMethod >= mySolNode.nar)
            anOrbit->addChild(setLineAttributesByParameterValue(
                    mySolNode.par[l-1][mySolNode.parID[coloringMethod-mySolNode.nar]],
                    mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                    mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                    mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                    stability, scaler));
        else
            anOrbit->addChild(setLineColorBlending(colorBase,
                mySolNode.numVerticesEachPeriod[l-1]*11,stability, type, scaler));
//    }
//    else
//    {
//        anOrbit->addChild(setLineAttributesByParameterValue(
//            curComponent, maxComponent, maxComponent/2.0, 0,
//            stability, scaler));
//    }

    anOrbit->addChild(tube.createTube());

    delete [] vertices;
    return anOrbit;
}


///////////////////////////////////////////////////////////////////////////
//
//         animate the solution by using lines. This version use less memory
//         and much faster.
//
SoSeparator *
drawABifBranchUsingLines(int iBranch, long int l, long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    int32_t  myint[10];

    SoSeparator * aBranch = new SoSeparator;
    long int size         = myBifNode.numVerticesEachBranch[l-1];
    float *colorBase      = new float[size];
    float (*vertices)[3]  = new float[size][3];

    long int curSize = 0;

    int lastStab = myBifNode.ptStability[si];
    int curStab  = lastStab;
    long int m   = 0;
    long int idx = si+m;
    do
    {
        lastStab = curStab;
        vertices[curSize][0] = myBifNode.xyzCoords[idx][0];
        vertices[curSize][1] = myBifNode.xyzCoords[idx][1];
        vertices[curSize][2] = myBifNode.xyzCoords[idx][2];
        if(coloringMethod >= 0)
            colorBase[curSize]  = clientData.bifData[idx][coloringMethod];
        else if(coloringMethod == CL_POINT_NUMBER)
            colorBase[curSize]  = m;
        else if(coloringMethod == CL_STABILITY)
            colorBase[curSize]  = myBifNode.ptStability[idx];
        m++;
        idx = si+m;
        if(m < size)
            curStab = myBifNode.ptStability[idx];
        curSize++;
        if( (lastStab == curStab || curSize <= 1) && m != size) continue;

        SoCoordinate3 *myCoords = new SoCoordinate3;
        myCoords->point.setValues(0, curSize, vertices);
        myint[0]= curSize;

        SoLineSet *myLine= new SoLineSet;
        myLine->numVertices.setValues(0, 1, myint);

        if(coloringMethod == CL_BRANCH_NUMBER)
            aBranch->addChild(setLineAttributesByBranch(iBranch, lastStab, scaler));
        else if(coloringMethod == CL_STABILITY)
            aBranch->addChild(setLineColorBlendingByStability(colorBase, curSize, lastStab, scaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            aBranch->addChild(setLineAttributesByType(lastStab, type, scaler));
        else
            aBranch->addChild(setLineColorBlending(colorBase, curSize,
                lastStab, type, scaler));

        aBranch->addChild(myCoords);
        aBranch->addChild(myLine);

        curSize = 0;
        vertices[curSize][0] = myBifNode.xyzCoords[idx-1][0];
        vertices[curSize][1] = myBifNode.xyzCoords[idx-1][1];
        vertices[curSize][2] = myBifNode.xyzCoords[idx-1][2];
        if(coloringMethod >= 0)
            colorBase[curSize]  = clientData.bifData[idx-1][coloringMethod];
        else if(coloringMethod == CL_POINT_NUMBER)
            colorBase[curSize]  = m-1;
        else if(coloringMethod == CL_STABILITY)
            colorBase[curSize]  = myBifNode.ptStability[idx-1];
        curSize++;
    }while( m < size);

    delete [] vertices;
    delete [] colorBase;
    return aBranch;
}


//////////////////////////////////////////////////////////////////////////
//          Draw a bif branch using NURBS curve
//
SoSeparator *
drawABifBranchUsingNurbsCurve(int iBranch, long int l,
long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    int32_t  myint[10];

    SoSeparator * aBranch = new SoSeparator;
    long int size = myBifNode.numVerticesEachBranch[l-1];
    float (*vertices)[3] = new float[size][3];
    float *colorBase = new float[size];
    for(long int m=0; m<size; ++m) 
    {
        long idx = si+m;
        vertices[m][0]=myBifNode.xyzCoords[si+m][0];
        vertices[m][1]=myBifNode.xyzCoords[si+m][1];
        vertices[m][2]=myBifNode.xyzCoords[si+m][2];
        if(coloringMethod>=0)
            colorBase[m]  =clientData.bifData[idx][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)
            colorBase[m]  = m;
    }
    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, size, vertices);
    myint[0]= size;

    SoGroup *cvGrp = new SoGroup;

    float * knots = new float[size+4];
    if(size> 4)
    {
        for (int i=0; i<4; ++i) knots[i]=0, knots[i+size]=size-3;
        for(int i=4; i<size; ++i) knots[i]=i-3;
        SoNurbsCurve *myCurve = new SoNurbsCurve;
        myCurve->numControlPoints = size;
        myCurve->knotVector.setValues(0, size+4, knots);
        cvGrp->addChild(myCurve);
    }
    else
    {
        SoLineSet *myLine= new SoLineSet;
        myLine->numVertices.setValues(0,1,myint);
        cvGrp->addChild(myLine);
    }

    if(coloringMethod == CL_BRANCH_NUMBER)
        aBranch->addChild(setLineAttributesByBranch(iBranch, stability, scaler));
    else if(coloringMethod == CL_STABILITY)
        aBranch->addChild(setLineAttributesByStability(stability, scaler));
    else if(coloringMethod == CL_ORBIT_TYPE)
        aBranch->addChild(setLineAttributesByType(stability, type, scaler));
    else
        aBranch->addChild(setLineColorBlending(colorBase, size,
            stability, type, scaler));
    aBranch->addChild(myCoords);
    aBranch->addChild(cvGrp);

    delete [] vertices;
    delete [] colorBase;
    delete [] knots;
    return aBranch;
}


//////////////////////////////////////////////////////////////////////////
//
//          Draw a label interval
//
SoSeparator *
drawABifLabelInterval(long int l, long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    int32_t  myint[10];

    SoSeparator * anInterval = new SoSeparator;
    long int size = myBifNode.numVerticesEachLabelInterval[l-1];
    float *colorBase = new float[size];
    float (*vertices)[3] = new float[size][3];
    for(int m=0; m<size; m++)
    {
        long int idx = si+m;
        vertices[m][0]=myBifNode.xyzCoords[idx][0];
        vertices[m][1]=myBifNode.xyzCoords[idx][1];
        vertices[m][2]=myBifNode.xyzCoords[idx][2];
        if(coloringMethod>=0)
            colorBase[m]  =clientData.bifData[idx][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)
            colorBase[m]  = m;
    }
    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, size, vertices);
    myint[0]=size;

    SoLineSet *myLine= new SoLineSet;
    myLine->numVertices.setValues(0,1,myint);

    if(coloringMethod == CL_ORBIT_TYPE)
        anInterval->addChild(setLineAttributesByType(stability, type, scaler));
    else
        anInterval->addChild(setLineColorBlending(colorBase, size,
            stability, type, scaler));

    anInterval->addChild(myCoords);
    anInterval->addChild(myLine);
    delete [] vertices;
    return anInterval;
}


//////////////////////////////////////////////////////////////////////////
//
//          Draw a label interval using NURBS curve
//
SoSeparator *
drawABifLabelIntervalUsingNurbsCurve(long int l,
      long int si, float scaler, int stability, int type)
//
//////////////////////////////////////////////////////////////////////////
{
    int32_t  myint[10];

    SoSeparator * anInterval = new SoSeparator;
    float (*vertices)[3];
    vertices = new float[myBifNode.numVerticesEachLabelInterval[l-1]][3];
    for(int m=0; m<myBifNode.numVerticesEachLabelInterval[l-1]; m++)
    {
        vertices[m][0]=myBifNode.xyzCoords[si+m][0];
        vertices[m][1]=myBifNode.xyzCoords[si+m][1];
        vertices[m][2]=myBifNode.xyzCoords[si+m][2];
    }
    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, myBifNode.numVerticesEachLabelInterval[l-1], vertices);
    myint[0]=myBifNode.numVerticesEachLabelInterval[l-1];

    SoGroup *cvGrp = new SoGroup;

    int number = myBifNode.numVerticesEachLabelInterval[l-1];
    float * knots = new float[number+4];
    if(number > 4)
    {
        for (int i=0; i<4; ++i) knots[i]=0, knots[i+number]=number-3;
        for(int i=4; i<number; ++i) knots[i]=i-3;
        SoNurbsCurve *myCurve = new SoNurbsCurve;
        myCurve->numControlPoints = number;
        myCurve->knotVector.setValues(0, number+4, knots);
        cvGrp->addChild(myCurve);
    }
    else
    {
        SoLineSet *myLine= new SoLineSet;
        myLine->numVertices.setValues(0,1,myint);
        cvGrp->addChild(myLine);
    }

    anInterval->addChild(setLineAttributesByType(stability, type, scaler));
    anInterval->addChild(myCoords);
    anInterval->addChild(cvGrp);

    delete [] vertices;
    delete [] knots;
    return anInterval;
}


///////////////////////////////////////////////////////////////////////////
//
// animate solution by using tubes. 
//
SoSeparator *
animateSolutionUsingTubes(bool aniColoring)
//
///////////////////////////////////////////////////////////////////////////
{
    long int sumX = 0;
    SoSeparator *solGroup = new SoSeparator;

    float dis = !options[OPT_NORMALIZE_DATA] ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0;

    SoBlinker *tubeBlker = new SoBlinker;
    tubeBlker->speed = orbitSpeed;
    int iBranch = 0;
    int curBranchID = mySolNode.branchID[iBranch];
    int sumOrbit    = mySolNode.numOrbitsInEachBranch[iBranch];
    for(int j=0; j<mySolNode.numOrbits; j++)
    {
        long int upperlimit = mySolNode.numVerticesEachPeriod[j];
        if(upperlimit == 1)
        {
            int idx = sumX;
            SoSeparator * ptSep = new SoSeparator;
            SoSphere * aPoint = new SoSphere;
            aPoint->radius = dis * STATIONARY_POINT_RADIUS;

            SoTransform * aTrans = new SoTransform;
            aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], \
                                         mySolNode.xyzCoords[idx][1], \
                                         mySolNode.xyzCoords[idx][2]);
            ptSep->addChild(aTrans);
            ptSep->addChild(aPoint);
            solGroup->addChild(ptSep);
        }
        else
        {
            float (*path)[3] = new float[upperlimit][3];
            float *colorBase = new float[upperlimit*11];
            int stability = clientData.labelIndex[j+1][3];
            int type = clientData.labelIndex[j+1][2];
            SoSeparator *anOrbit = new SoSeparator;
            Tube tube;
            if(j >= sumOrbit)
            {
                curBranchID = mySolNode.branchID[++iBranch];
                sumOrbit   += mySolNode.numOrbitsInEachBranch[iBranch];
            }

            for(int i=0; i<upperlimit; i++)
            {
                int idx = i+sumX;
                path[i][0]=mySolNode.xyzCoords[idx][0];
                path[i][1]=mySolNode.xyzCoords[idx][1];
                path[i][2]=mySolNode.xyzCoords[idx][2];
                if(coloringMethod>=0)
                    for(int j=0; j<11; ++j)
                        colorBase[i*11+j]  = clientData.solData[idx][coloringMethod];
                if(coloringMethod==CL_POINT_NUMBER)
                    for(int j=0; j<11; ++j)
                        colorBase[i*11+j]  = i;
            }

            if(!aniColoring)
                anOrbit->addChild(setLineAttributesByType(stability, 0, lineWidthScaler));
            else if(coloringMethod == CL_STABILITY)
                anOrbit->addChild(setLineAttributesByStability(stability, lineWidthScaler));
            else if(coloringMethod == CL_BRANCH_NUMBER)
                anOrbit->addChild(setLineAttributesByBranch(iBranch, stability, lineWidthScaler));
            else if(coloringMethod == CL_ORBIT_TYPE)
                anOrbit->addChild(setLineAttributesByType(stability, type, lineWidthScaler));
            else if(coloringMethod == CL_LABELS)
            {
/*
                double bMin = 0;
                for(int ib = 0; ib< iBranch; ++ib)
                    bMin +=  mySolNode.numOrbitsInEachBranch[ib];
                double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
                anOrbit->addChild(setLineAttributesByParameterValue(
                    j, bMax, (bMax+bMin)/2.0, bMin,
                    stability, lineWidthScaler));
*/
                anOrbit->addChild(setLineAttributesByParameterValue(
                    j, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                    stability, lineWidthScaler));
            }
            else if(coloringMethod >= mySolNode.nar)
                anOrbit->addChild(setLineAttributesByParameterValue(
                        mySolNode.par[j][mySolNode.parID[coloringMethod-mySolNode.nar]],
                        mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                        mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                        mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                        stability, lineWidthScaler));
            else if(coloringMethod == CL_COMPONENT)
            {
                 anOrbit->addChild(setLineAttributesByParameterValue(
                     curComponent, maxComponent, maxComponent/2.0, 0,
                     stability, lineWidthScaler));
            }
            else
                anOrbit->addChild(setLineColorBlending(colorBase,
                    upperlimit*11,stability, type, lineWidthScaler));

            tube = Tube(upperlimit, path, lineWidthScaler*0.0075, 10);
            anOrbit->addChild(tube.createTube());
            tubeBlker->addChild(anOrbit);  
            delete [] path;
            delete [] colorBase;
        }
        sumX += upperlimit;
    }
    solGroup->addChild(tubeBlker);
    return solGroup;
}


///////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawATube(TubeNode cnode)
//
///////////////////////////////////////////////////////////////////////////
{
    SoSeparator *tSep = new SoSeparator;
    SoTransform *tXform = new SoTransform;
    tXform->translation.setValue(cnode.translation.x,\
        cnode.translation.y,\
        cnode.translation.z);
    tXform->rotation.setValue(SbVec3f(cnode.axis.x,\
        cnode.axis.y,\
        cnode.axis.z),\
        cnode.angle);
    SoCylinder *tCyl = new SoCylinder;
    tCyl->radius = 0.005;
    tCyl->height = cnode.height;
    tCyl->parts = SoCylinder::SIDES;

    tSep->addChild(tXform);
    tSep->addChild(tCyl);
    return tSep;
}


///////////////////////////////////////////////////////////////////////////
//
SoSeparator *
animateOrbitWithTail(int iBranch, long int j, long int si)
//
///////////////////////////////////////////////////////////////////////////
{
   
    SoSeparator *aniSep = new SoSeparator;
    SoSeparator *satGroup = new SoSeparator;

    float distance = !options[OPT_NORMALIZE_DATA] ? (max(max(fabs(mySolNode.max[0]-mySolNode.min[0]),
        fabs(mySolNode.max[1]-mySolNode.min[1])),
        fabs(mySolNode.max[2]-mySolNode.min[2]))) : 2.0;
    int stability = clientData.labelIndex[j][3];
    int type = clientData.labelIndex[j][2];

    long int upperlimit = mySolNode.numVerticesEachPeriod[j-1];
    long int idx = si; 
    if(upperlimit == 1)
    {
        SoSeparator * ptSep = new SoSeparator;
        SoTransform * aTrans = new SoTransform;
        aTrans->translation.setValue(mySolNode.xyzCoords[idx][0], 
                                     mySolNode.xyzCoords[idx][1], 
                                     mySolNode.xyzCoords[idx][2]);
        ptSep->addChild(aTrans);

        SoSphere * aPoint = new SoSphere;
        aPoint->radius = distance * STATIONARY_POINT_RADIUS;
        ptSep->addChild(aPoint);
        satGroup->addChild(ptSep);
        return satGroup;
    }

    float maxV[3], minV[3];
    long int orbitSize =  upperlimit;
    long int arrSize = orbitSize;

    double *time = new double[upperlimit+1];
    double dt = 1.0/upperlimit;

    maxV[0]=minV[0]=mySolNode.xyzCoords[idx+0][0];
    maxV[1]=minV[1]=mySolNode.xyzCoords[idx+0][0];
    maxV[2]=minV[2]=mySolNode.xyzCoords[idx+0][0];

    time[0] = 0.0;
    for(long int i=1; i<upperlimit; i++)
    {
        if(maxV[0]<mySolNode.xyzCoords[idx+i][0]) maxV[0]=mySolNode.xyzCoords[idx+i][0] ;
        if(minV[0]>mySolNode.xyzCoords[idx+i][0]) minV[0]=mySolNode.xyzCoords[idx+i][0] ;
        if(maxV[1]<mySolNode.xyzCoords[idx+i][1]) maxV[1]=mySolNode.xyzCoords[idx+i][1] ;
        if(minV[1]>mySolNode.xyzCoords[idx+i][1]) minV[1]=mySolNode.xyzCoords[idx+i][1] ;
        if(maxV[2]<mySolNode.xyzCoords[idx+i][2]) maxV[2]=mySolNode.xyzCoords[idx+i][2] ;
        if(minV[2]>mySolNode.xyzCoords[idx+i][2]) minV[2]=mySolNode.xyzCoords[idx+i][2] ;
        time[i] = i*dt;
    }

    float dis = distance;// fabs(max(max((maxV[0]-minV[0]), (maxV[1]-minV[1])), (maxV[2]-minV[2])));
    float (*myVertices)[3]= new float[arrSize+1][3];
    float *myColorBase = new float [arrSize+1];

    myVertices[0][0] = myVertices[arrSize][0] = mySolNode.xyzCoords[idx][0];
    myVertices[0][1] = myVertices[arrSize][1] = mySolNode.xyzCoords[idx][1];
    myVertices[0][2] = myVertices[arrSize][2] = mySolNode.xyzCoords[idx][2];
    if(coloringMethod>=0)myColorBase[0]  = clientData.solData[idx][coloringMethod];
    if(coloringMethod==CL_POINT_NUMBER)myColorBase[0]  = 0;
    for(long int i=1; i<upperlimit; i++)
    {
        double tTemp = time[i];
        long int m = 0;
        while(tTemp > mySolNode.time[idx+m] && m < upperlimit) ++m;

        if( fabs(tTemp-mySolNode.time[idx+m]) <= 1.0e-9 || \
            fabs(mySolNode.time[idx+m]-mySolNode.time[idx+m-1]<=1.0e-8))
        {
            myVertices[i][0] = mySolNode.xyzCoords[idx+m][0];
            myVertices[i][1] = mySolNode.xyzCoords[idx+m][1];
            myVertices[i][2] = mySolNode.xyzCoords[idx+m][2];
        }
        else
        {
            myVertices[i][0] = (mySolNode.xyzCoords[idx+m][0] + \
                                mySolNode.xyzCoords[idx+m-1][0])*0.5;
            myVertices[i][1] = (mySolNode.xyzCoords[idx+m][1] + \
                                mySolNode.xyzCoords[idx+m-1][1])*0.5;
            myVertices[i][2] = (mySolNode.xyzCoords[idx+m][2] + \
                                mySolNode.xyzCoords[idx+m-1][2])*0.5;
        }

        if(coloringMethod>=0)myColorBase[i]  = clientData.solData[idx+m][coloringMethod];
        if(coloringMethod==CL_POINT_NUMBER)myColorBase[i]  = i;
    }

    SoDrawStyle *satStyle = new SoDrawStyle;
    satStyle->style = SoDrawStyle::FILLED;
    satGroup->addChild(satStyle);

    SoCoordinate3 *myCoords = new SoCoordinate3;
    myCoords->point.setValues(0, arrSize+1, myVertices);
    satGroup->addChild(myCoords);

    
    SoTimeCounter *myCounter = new SoTimeCounter;
    myCounter->max = arrSize+1;
    myCounter->min = 0;
   
    float freq, fduty; 

    if ( numPeriodAnimated == -1 ) 
    {
         freq =  0.1*satSpeed/128;
         fduty = 127*arrSize;
    }
    else if( numPeriodAnimated == 0 )
    {
         freq = 0;
         fduty = 0;
    }
    else if ( numPeriodAnimated == 1 ){
         freq = 0.1*satSpeed; 
         fduty = 1;
    }
    else {
         freq =  0.1*satSpeed/numPeriodAnimated;
         fduty = (numPeriodAnimated-1)*arrSize;
    }

    myCounter->frequency =  freq ;

    int iduty = 0;
    for(iduty = 0; iduty <= arrSize+1; ++iduty)
         if(iduty == arrSize+1) myCounter->duty.set1Value(iduty, fduty) ;
         else myCounter->duty.set1Value(iduty, 1) ;

//------------------------------------------Begin-----------------------------------------
    float scaler = lineWidthScaler;

//    if(maxComponent == 1)
//    {
//------------------------------------------End-----------------------------------------
        if(coloringMethod == CL_BRANCH_NUMBER)
            satGroup->addChild(setLineAttributesByBranch(iBranch, stability, lineWidthScaler));
        else if(coloringMethod == CL_STABILITY)
            satGroup->addChild(setLineAttributesByStability(stability, lineWidthScaler));
        else if(coloringMethod == CL_ORBIT_TYPE)
            satGroup->addChild(setLineAttributesByType(stability, type, lineWidthScaler));
        else if(coloringMethod == CL_LABELS)
        {
/*
            double bMin = 0;
            for(int ib = 0; ib< iBranch; ++ib)
                bMin +=  mySolNode.numOrbitsInEachBranch[ib];
            double bMax = bMin+mySolNode.numOrbitsInEachBranch[iBranch]-1;
            satGroup->addChild(setLineAttributesByParameterValue(
                j-1, bMax, (bMax+bMin)/2.0, bMin,
                stability, lineWidthScaler));
*/
            satGroup->addChild(setLineAttributesByParameterValue(
                j-1, clientData.totalLabels, clientData.totalLabels/2.0, 0,
                stability, lineWidthScaler));
        }
        else if(coloringMethod == CL_COMPONENT)
            satGroup->addChild(setLineAttributesByParameterValue(
                curComponent, maxComponent, maxComponent/2.0, 0,
                stability, scaler));
        else if(coloringMethod >= mySolNode.nar)
        {
            satGroup->addChild(setLineAttributesByParameterValue(
                mySolNode.par[j-1][mySolNode.parID[coloringMethod-mySolNode.nar]],
                mySolNode.parMax[iBranch][coloringMethod-mySolNode.nar],
                mySolNode.parMid[iBranch][coloringMethod-mySolNode.nar],
                mySolNode.parMin[iBranch][coloringMethod-mySolNode.nar],
                stability, lineWidthScaler));
        }
        else
        {
            satGroup->addChild(setLineColorBlending(myColorBase, arrSize,
                stability, type, lineWidthScaler));
        }
//------------------------------------------Begin-----------------------------------------
//    }
//    else
//    {
//        satGroup->addChild(setLineAttributesByParameterValue(
//            curComponent, maxComponent, maxComponent/2.0, 0,
//            stability, scaler));
//    }
//------------------------------------------End-----------------------------------------

    SoLineSet *myLine= new SoLineSet;
    myLine->numVertices.connectFrom(&myCounter->output);
    satGroup->addChild(myLine);

    SoMaterial * satMtl = new SoMaterial;
    SoSphere * mySat = new SoSphere;
    mySat->radius = satRadius*dis*0.005;


    SoTranslation * satTrans = new SoTranslation;
//    satMtl->diffuseColor.setValue(envColors[4]);

//    satGroup->addChild(satMtl);
    satGroup->addChild(satTrans);
    satGroup->addChild(mySat);

    SoTimeCounter *myCounter2 = new SoTimeCounter;
    myCounter2->max = arrSize;
    myCounter2->min = 0;
    if (numPeriodAnimated != 0)
        myCounter2->frequency =  0.1*satSpeed;//*numPeriodAnimated;
    else
        myCounter2->frequency =  0.1*satSpeed;
    for(iduty = 0; iduty < arrSize; ++iduty)
         myCounter2->duty.set1Value(iduty, 1) ;

    SoSelectOne *mysel = new SoSelectOne(SoMFVec3f::getClassTypeId());
    mysel->index.connectFrom(&myCounter2->output);
    mysel->input->enableConnection(TRUE);
    mysel->input->connectFrom(&myCoords->point);
    satTrans->translation.connectFrom(mysel->output);

    delete [] myVertices;
    delete [] myColorBase;
    delete [] time;
    return satGroup;
}


///////////////////////////////////////////////////////////////////////////
//
SoSeparator *
animateOrbitMovement(long int j, long int si)
//
///////////////////////////////////////////////////////////////////////////
{
    float ptb[3], pts[4][3];
    SoSeparator *satGroup = new SoSeparator;

    SoMaterial *satMtl = new SoMaterial;
    satMtl->diffuseColor.setValue(1.0,0.0,0.0);
    satMtl->transparency = 0.0;

    SoDrawStyle *satStyle = new SoDrawStyle;
    satStyle->style = SoDrawStyle::FILLED;
    satGroup->addChild(satStyle);

    SoSeparator * satSep = new SoSeparator;
    SoBlinker *satBlker = new SoBlinker;

    int upperlimit = mySolNode.numVerticesEachPeriod[j-1];
    int idx = si;             
    satBlker->speed = satSpeed;

    float maxV[3], minV[3];
    maxV[0]=minV[0]=mySolNode.xyzCoords[idx+0][0];
    maxV[1]=minV[1]=mySolNode.xyzCoords[idx+0][0];
    maxV[2]=minV[2]=mySolNode.xyzCoords[idx+0][0];
    double *time = new double[upperlimit];
    float dt = 1.0/upperlimit;
    time[0] = 0.0;
    for(int i=1; i<upperlimit; i++)
    {
        if(maxV[0]<mySolNode.xyzCoords[idx+i][0]) maxV[0]=mySolNode.xyzCoords[idx+i][0] ;
        if(minV[0]>mySolNode.xyzCoords[idx+i][0]) minV[0]=mySolNode.xyzCoords[idx+i][0] ;
        if(maxV[1]<mySolNode.xyzCoords[idx+i][1]) maxV[1]=mySolNode.xyzCoords[idx+i][1] ;
        if(minV[1]>mySolNode.xyzCoords[idx+i][1]) minV[1]=mySolNode.xyzCoords[idx+i][1] ;
        if(maxV[2]<mySolNode.xyzCoords[idx+i][2]) maxV[2]=mySolNode.xyzCoords[idx+i][2] ;
        if(minV[2]>mySolNode.xyzCoords[idx+i][2]) minV[2]=mySolNode.xyzCoords[idx+i][2] ;
        time[i] = i*dt;
    }
    float dis = max(max((maxV[0]-minV[0]), (maxV[1]-minV[1])), (maxV[2]-minV[2]));

    ptb[0]=mySolNode.xyzCoords[idx][0];
    ptb[1]=mySolNode.xyzCoords[idx][1];
    ptb[2]=mySolNode.xyzCoords[idx][2];
    satBlker->addChild(drawASphere(ptb,1.*dis/100.0));

    for(int i=1; i<upperlimit; i++)
    {
        int m = 1;
        while(time[i] > mySolNode.time[idx+m] && m < upperlimit) ++m;

        if( fabs(time[i]-mySolNode.time[m]) <= 1.0e-9 )
        {
            ptb[0]=mySolNode.xyzCoords[idx+m][0];
            ptb[1]=mySolNode.xyzCoords[idx+m][1];
            ptb[2]=mySolNode.xyzCoords[idx+m][2];
            satBlker->addChild(drawASphere(ptb,1.*dis/100.0));
        }
        else
        {
            ptb[0]= (mySolNode.xyzCoords[idx+m][0]+mySolNode.xyzCoords[idx+m-1][0])/2.0;
            ptb[1]= (mySolNode.xyzCoords[idx+m][1]+mySolNode.xyzCoords[idx+m-1][1])/2.0;
            ptb[2]= (mySolNode.xyzCoords[idx+m][2]+mySolNode.xyzCoords[idx+m-1][2])/2.0;
            satBlker->addChild(drawASphere(ptb,1.*dis/100.0));
        }
    }
    delete [] time;
    satGroup->addChild(satMtl);
    satGroup->addChild(satBlker);
    return satGroup;
}


///////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawASphere(float position[], float size)
//
///////////////////////////////////////////////////////////////////////////
{
    float scaler = 1.0e-4;
    SoSeparator *satSep = new SoSeparator;
    SoTransform *satXform = new SoTransform;

    satXform->translation.setValue(position[0],position[1],position[2]);
    satSep->addChild(satXform);

    SoSeparator *satSph;

    bool obj = FALSE;
    SoInput mySceneInput;
    if (obj && mySceneInput.openFile("widgets/sattelite.iv"))
    {
        satSph = SoDB::readAll(&mySceneInput);
        if (satSph == NULL)
        {
            obj=FALSE;
        }
        else
        {

            mySceneInput.closeFile();
            SoTransform *satTrans = new SoTransform;
            satTrans->scaleFactor.setValue(scaler,scaler,scaler);
            satSep->addChild(satTrans);
            satSep->addChild(satSph);
            satSep->addChild(satSph);
            obj = TRUE;
        }
    }
    else
    {
        obj = FALSE;
    }

    if(!obj)
    {
        SoSphere *satSph = new SoSphere;
        satSph->radius = size;
        satSep->addChild(satSph);
    }

    return satSep;
}


///////////////////////////////////////////////////////////////////////////
//
SoSeparator *
drawASphereWithColor(float color[], float position[], float size)
//
///////////////////////////////////////////////////////////////////////////
{
    SoSeparator *satSep = new SoSeparator;
    SoTransform *satXform = new SoTransform;
    satXform->translation.setValue(position[0],position[1],position[2]);

    SoMaterial *satMtl = new SoMaterial;
    satMtl->diffuseColor.setValue(color);
    satMtl->transparency = 0.0;
    satSep->addChild(satMtl);

    SoSphere *satSph = new SoSphere;
    satSph->radius = size;

    satSep->addChild(satXform);
    satSep->addChild(satSph);

    return satSep;
}


////////////////////////////////////////////////////////////////////////
//
//  initialize win basic things
//
bool
readSolutionAndBifurcationData(bool blFirstRead)
//
///////////////////////////////////////////////////////////////////////
{
    long int  total=0, rows=0;

    solHead = parseSolution(sFileName, blOpenSolFile, total, rows);
    if(!blOpenSolFile)
        printf(" Solution file does not exist.\n");

    mySolNode.time = new double[mySolNode.totalNumPoints];
    mySolNode.xyzCoords = new float[mySolNode.totalNumPoints][3];
    mySolNode.numAxis   = 3;

    clientData.solMax = new float [mySolNode.nar+1];
    clientData.solMin = new float [mySolNode.nar+1];

    clientData.solData = new float*[mySolNode.totalNumPoints];
    for(int ml=0; ml<mySolNode.totalNumPoints; ++ml)
        clientData.solData[ml]= new float [mySolNode.nar];

    blOpenBifFile = parseBifurcation(bFileName) ? true : false;
    if(!blOpenBifFile) printf(" Bifurcation file does not exist!\n");

    if((!blOpenBifFile) && (!blOpenSolFile) && (!blFirstRead)) return false;
    else if((!blOpenBifFile) && (!blOpenSolFile))
    {
//        printf(" Target files do not exist!\n");
        exit(1);
    }

    myBifNode.xyzCoords = new float[myBifNode.totalNumPoints][3];
    myBifNode.ptStability = new int[myBifNode.totalNumPoints];

    clientData.bifData = new float*[myBifNode.totalNumPoints];
    for(int ml=0; ml<myBifNode.totalNumPoints; ++ml)
        clientData.bifData[ml]= new float [myBifNode.nar];


    clientData.multipliers = new float[myBifNode.totalNumPoints][6][2];

    int varIndices[3];
    if( blOpenBifFile)
    {
        bool tmp = false;
        tmp = readBifurcation(bFileName, varIndices) ? true : false;
        if(!tmp) printf(" Failed to read the bifurcation file!\n");
    }
    else
    {
        whichType = SOLUTION;
    }

    if( blOpenSolFile )
    {
        bool tmp = false;
        tmp = readSolution(solHead, sFileName, varIndices) ? true : false;
        if(!tmp) printf(" Failed to read the solution file!\n");
        blOpenSolFile = tmp;

        if(mySolNode.nar <= 3) setShow3D = false;
    }
    else
    {
        whichType = BIFURCATION;
    }

    if((!blOpenBifFile) && (!blOpenSolFile))
    {
//        printf(" Target files do not exist!\n");
        exit(1);
    }

    int st = readFM(dFileName, myBifNode.totalNumPoints);
    if(st!=0)
//        printf(" D file OK.\n");
//    else
        printf(" Failed to read the diagnostic file.\n");

    return TRUE;
}


////////////////////////////////////////////////////////////////////////
//
//
//
void
copySolDataToWorkArray(int  varIndices[])
//
////////////////////////////////////////////////////////////////////////
{
    double mx, mi;
    for(int k=0; k<3; k++)
    {
        for(long int row=0; row<mySolNode.totalNumPoints; ++row)
        {
            mySolNode.time[row] = clientData.solData[row][0];
            if(varIndices[k]>=0)
            {
                float dummy = clientData.solData[row][varIndices[k]];
                mySolNode.xyzCoords[row][k] = dummy;
/*
                if(dummy>mySolNode.max[k] || row==0 )
                    mySolNode.max[k] = dummy;
                if(dummy<mySolNode.min[k] || row==0 )
                    mySolNode.min[k] = dummy;
*/
            }
            else if(varIndices[k]<0)
            {
                mySolNode.xyzCoords[row][k]=0.0;
//                mySolNode.max[k]= 1;
//                mySolNode.min[k]=-1;
            }
        }
/*
        mx = mySolNode.max[k];
        mi = mySolNode.min[k];
        rounding(mx, mi);
        mySolNode.max[k] = mx;
        mySolNode.min[k] = mi;
*/
    }
}

////////////////////////////////////////////////////////////////////////
//
//
//
void
searchForMaxMin(int component, int  varIndices[])
//
////////////////////////////////////////////////////////////////////////
{
    double mx, mi;
    for(int k=0; k<3; k++)
    {
        for(long int row=0; row<mySolNode.totalNumPoints; ++row)
        {
            if(varIndices[k]>=0)
            {
                float dummy = clientData.solData[row][varIndices[k]];
                if(dummy>mySolNode.max[k] || (row==0 && component==1))
                    mySolNode.max[k] = dummy;
                if(dummy<mySolNode.min[k] || (row==0 && component==1))
                    mySolNode.min[k] = dummy;
            }
            else if(varIndices[k]<0)
            {
                mySolNode.max[k]= 1;
                mySolNode.min[k]=-1;
            }
        }
        mx = mySolNode.max[k];
        mi = mySolNode.min[k];
        rounding(mx, mi);
        mySolNode.max[k] = mx;
        mySolNode.min[k] = mi;
    }
}

////////////////////////////////////////////////////////////////////////
//
//
//
void
copyBifDataToWorkArray(int  varIndices[])
//
////////////////////////////////////////////////////////////////////////
{

    double mx, mi;

    for(int k=0; k<3; k++)
    {
        for(long int row=0; row<myBifNode.totalNumPoints; ++row)
        {
            if(varIndices[k]>=0)
            {
                float dummy = clientData.bifData[row][varIndices[k]];

                myBifNode.xyzCoords[row][k] = dummy;
                if(dummy>myBifNode.max[k] || row==0 )
                    myBifNode.max[k] = dummy;
                if(dummy<myBifNode.min[k] || row==0 )
                    myBifNode.min[k] = dummy;
            }
            else if(varIndices[k]<0)
            {
                myBifNode.xyzCoords[row][k]=0.0;
                myBifNode.max[k]= 1;
                myBifNode.min[k]=-1;
            }
        }

        mx = myBifNode.max[k];
        mi =myBifNode.min[k];
        rounding(mx, mi);
        myBifNode.max[k] = mx;
        myBifNode.min[k] = mi;
    }
}



////////////////////////////////////////////////////////////////////////
//
void
lookForThePoint(float position[],long int &bIdx, long int &sIdx)
//
////////////////////////////////////////////////////////////////////////
{

    int varIndices[3];
    int mx = max(max(xCoordIdxSize, yCoordIdxSize), max(yCoordIdxSize, zCoordIdxSize));
    float minDis = 10000;
    long int index = 0;
    long int ib = 0;
    float distance;
    for(int i=0; i<mx; i++)
    {
        varIndices[0]=xCoordIndices[(i>=xCoordIdxSize)?(i%xCoordIdxSize):(i)];
        varIndices[1]=yCoordIndices[(i>=yCoordIdxSize)?(i%yCoordIdxSize):(i)];
        varIndices[2]=zCoordIndices[(i>=zCoordIdxSize)?(i%zCoordIdxSize):(i)];
        animationLabel = myLabels[lblIndices[0]];
        long int lblidx = lblIndices[0];
        if(whichType != BIFURCATION)
        {
            if(animationLabel == 0 || lblIdxSize >1)
            {
                float p1[3];
                for(long int j=0; j<mySolNode.totalNumPoints; ++j)
                {
                    for(int k=0; k<3; ++k)
                        p1[k] = clientData.solData[j][varIndices[k]];
                    distance = 0;
                    for(int k=0; k<3; ++k)
                        distance += (position[k]-p1[k])*(position[k]-p1[k]);
                    if(minDis > distance)
                    {
                        minDis = distance;
                        index  = j;
                    }
                }

                long int sumup = 0;
                ib = 0;
                while (sumup <index && ib<clientData.totalLabels)
                    sumup += mySolNode.numVerticesEachPeriod[ib++];
            }
            else
            {
                long int sumup = 0;
                float p1[3];
                for(int j=1; j<lblidx; ++j)
                {
                    sumup += mySolNode.numVerticesEachPeriod[j-1];
                }
                for(long int j=0; j<mySolNode.numVerticesEachPeriod[lblidx-1]; ++j)
                {
                    for(int k=0; k<3; ++k)
                        p1[k] = clientData.solData[sumup+j][varIndices[k]];
                    distance = 0;
                    for(int k=0; k<3; ++k)
                        distance += (position[k]-p1[k])*(position[k]-p1[k]);
                    if(minDis > distance)
                    {
                        minDis = distance;
                        index  = j+sumup;
                    }
                }
                ib = lblidx;  
            }
            sIdx = index;
            bIdx = clientData.labelIndex[ib][1];
        }
        else
        {
            {
                float p1[3];
                for(int j=0; j<myBifNode.totalNumPoints; ++j)
                {
                    for(int k=0; k<3; ++k)
                        p1[k] = clientData.bifData[j][varIndices[k]];
                    distance = 0;
                    for(int k=0; k<3; ++k)
                        distance += (position[k]-p1[k])*(position[k]-p1[k]);
                    if(minDis > distance)
                    {
                        minDis = distance;
                        index  = j;
                    }
                }
            }
            bIdx = index;
        }
    }
}


///////////////////////////////////////////////////////////////////////////
//
void
pointsToAxisAngle(float * a, float * b, TubeNode &value)
//
///////////////////////////////////////////////////////////////////////////
{
    float c[3]={0.,0.,0.};
    float normc[3];
    int i=0;
    value.translation.x=value.translation.y=value.translation.z=0.0;
    value.axis.x=value.axis.y=value.axis.z=0.0;
    value.angle=0.0;
    value.height=0.0;
    float scaler = 2.0;
    for(i=0; i<3; i++)
        c[i]=b[i]-a[i];
    if(c[1]<0)
    {
        value.translation.x=b[0];
        value.translation.y=b[1];
        value.translation.z=b[2];
        scaler = 2.0;
        for(i=0; i<3; i++)
            c[i]=-c[i];
    }
    else
    {
        value.translation.x=a[0];
        value.translation.y=a[1];
        value.translation.z=a[2];
    }

    float c_norm, axis_norm;
    c_norm=sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
    value.height=c_norm*scaler; 
    if(c_norm>0.0000001)
    {
        for(i=0; i<3; i++)
            normc[i]=0.0;
        for(i=0; i<3; i++)
            normc[i]=c[i]/c_norm;
        value.axis.x=normc[2];
        value.axis.y=0.0;
        value.axis.z=-normc[0];
        axis_norm=sqrt(value.axis.x*value.axis.x +\
            value.axis.y*value.axis.y +\
            value.axis.z*value.axis.z);
        if(axis_norm < 0.00000001)
        {
            value.angle=0.0;
        }
        else
        {
            value.angle=asin(axis_norm);
        }
    }
}


////////////////////////////////////////////////////////////////////////
//
void
showAboutCB(Widget dialog, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XtUnmanageChild (dialog);
}


////////////////////////////////////////////////////////////////////////
//
void 
hidenDialogShell (Widget widget, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XtUnmanageChild (XtParent(widget));
}


////////////////////////////////////////////////////////////////////////
//
void 
redrawFloqueMultipliers (Widget fmDrawingArea, XtPointer client_data, XtPointer call_data)
//
////////////////////////////////////////////////////////////////////////
{
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) call_data;

    XPoint points[8]={ 0,0, 100, 0, 0,100, 100,100};

    XAllocNamedColor(cbs->event->xexpose.display, colormap, "blue", &blue, &exact);
    XAllocNamedColor(cbs->event->xexpose.display, colormap, "green", &green, &exact);
    XAllocNamedColor(cbs->event->xexpose.display, colormap, "white", &white, &exact);
    XAllocNamedColor(cbs->event->xexpose.display, colormap, "red", &red, &exact);
    XAllocNamedColor(cbs->event->xexpose.display, colormap, "black", &black, &exact);
    XAllocNamedColor(cbs->event->xexpose.display, colormap, "grey", &grey, &exact);

    static XTextItem myText[9] =
    {
        { "-inf", 4, 0, None }
        ,
        { "-100", 4, 0, None }
        ,
        { "-10", 3, 0, None },
        {"-1", 2, 0, None},
        { "0", 1, 0, None }
        ,
        { "1", 1, 0, None },
        {"10", 2, 0, None},
        { "100", 3, 0, None }
        ,
        { "+inf", 4, 0, None }
    };

    XSetForeground(cbs->event->xexpose.display, gc, blue.pixel);

    XSetLineAttributes(cbs->event->xexpose.display, gc, 2, LineSolid, CapRound, JoinRound);
    XDrawLine(cbs->event->xexpose.display, cbs->window, gc, 200, 0, 200, 400);

    XSetForeground(cbs->event->xexpose.display, gc, red.pixel);
    XSetLineAttributes(cbs->event->xexpose.display, gc, 2, LineSolid, CapRound, JoinRound);
    XDrawLine(cbs->event->xexpose.display, cbs->window, gc, 0, 200, 400, 200);

    XSetForeground(cbs->event->xexpose.display, gc, grey.pixel);
    XSetLineAttributes(cbs->event->xexpose.display, gc, 1, LineOnOffDash, CapButt, JoinRound);
    for(int i=0; i<9; ++i)
        XDrawLine(cbs->event->xexpose.display, cbs->window, gc, 0, 50*i, 400, 50*i);
    XSetLineAttributes(cbs->event->xexpose.display, gc, 1, LineOnOffDash, CapButt, JoinRound);
    for(int i=0; i<9; ++i)
        XDrawLine(cbs->event->xexpose.display, cbs->window, gc, i*50, 0, i*50, 400);

    XSetForeground(cbs->event->xexpose.display, gc, black.pixel);
    for(int i = 0; i < 9; ++i)
        XDrawText(cbs->event->xexpose.display,cbs-> window, gc, i*50-3 , 215, &myText[i], 1);
    for(int i = 0; i < 9; ++i)
        XDrawText(cbs->event->xexpose.display,cbs-> window, gc, 210 , 410-i*50, &myText[i], 1);

    XSetForeground(cbs->event->xexpose.display, gc, green.pixel);

    XSetLineAttributes(cbs->event->xexpose.display, gc, 1, LineSolid, CapRound, JoinRound);
    XDrawArc(cbs->event->xexpose.display, cbs->window, gc, 150, 150, 100, 100, 360*64, 360*64);

    XSetForeground(cbs->event->xexpose.display, gc, black.pixel);

    int x, y;

    XSetLineAttributes(cbs->event->xexpose.display, gc, 2, LineSolid, CapRound, JoinRound);
    for(int j = 0; j<clientData.numFM; ++j)
    {
        float tmp = fmData[2*j];
        if(fabs(tmp) <= 1.1)
            x = (tmp>0.0) ? (int)(200+tmp*50) : (int)(200-fabs(tmp)*50);
        else
            x =(tmp>0.0) ? (int)(250+log10(tmp)*50) : (int)(150-log10(fabs(tmp))*50);

        tmp = fmData[2*j+1];
        if(fabs(tmp) <= 1.1)
            y = (tmp>0.0) ? (int)(200+tmp*50) : (int)(200-fabs(tmp)*50);
        else
            y =(tmp>0.0) ? (int)(150-log10(tmp)*50): (int)(log10(fabs(tmp))*50+250);

        if(x>390) x = 390; if(x<10) y = 10;
        if(y>390) y = 390; if(y<10) y = 10;

        XDrawLine(cbs->event->xexpose.display, cbs->window, gc, x-3, y-3, x+3, y+3);
        XDrawLine(cbs->event->xexpose.display, cbs->window, gc, x-3, y+3, x+3, y-3);

    }
}


////////////////////////////////////////////////////////////////////////
//
void
popupFloquetMultiplierDialog(float data[], int size)
//
////////////////////////////////////////////////////////////////////////
{

    static Widget dialog_shell = (Widget) 0 ;
    Widget pane = (Widget) 0 ;              
    XmString      str1;
    void          showAboutCB(Widget, XtPointer, XtPointer);
    unsigned char modality = (unsigned char)XmDIALOG_FULL_APPLICATION_MODAL;
    char *str, temp[200];

    str = new char[size*50];
    str[0]='\0';

    for(int i=0; i<size; ++i)
    {
        strcat(str," Col[");
        sprintf(temp,"%2d",i+1);
        strcat(str,temp);
        strcat(str," ] = ");

        sprintf(temp,"%+E",data[i]);
        strcat(str,temp);
        if(size<20 || (size>=20 && (i+1)%2==0)) strcat(str,"\n");
        else strcat(str," | ");
    }

    char *tmpstr, tempchar[500];
    tmpstr = new char[500];
    tmpstr[0]='\0';
    strcat(tmpstr,"Floquet multipliers:\n" );
    for(int j=0; j<clientData.numFM; ++j)
    {
        strcat(tmpstr," [");
        sprintf(temp,"%2d",j);
        strcat(tmpstr,temp);
        strcat(tmpstr,"] : ");
        sprintf(temp,"%E",fmData[j*2]);
        strcat(tmpstr,temp);
        strcat(tmpstr," , ");
        sprintf(temp,"%E",fmData[j*2+1]);
        strcat(tmpstr,temp);
        strcat(tmpstr,"\n");
    }

    if (!pane)
    {
        Widget fmDrawingArea;
        Arg args[5];
        Pixel fg,bg;
        int n=0;
        XtSetArg(args[n], XmNdeleteResponse,XmDESTROY); ++n;
        dialog_shell = XmCreateDialogShell(topform, "Dialog", args, n);

        n=0;
        XtSetArg(args[n], XmNsashWidth, 1); ++n;
        XtSetArg(args[n], XmNsashHeight, 2); ++n;
        pane = XmCreatePanedWindow(dialog_shell, "pane",args,n);

        n = 0;
        XtSetArg (args[n], XmNunitType, Xm1000TH_INCHES); n++;
        XtSetArg (args[n], XmNwidth,  5500); n++;
        XtSetArg (args[n], XmNheight, 5500); n++;
        XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
        fmDrawingArea = XmCreateDrawingArea (pane, "fmDrawingArea", args, n);

        XtAddCallback (fmDrawingArea, XmNexposeCallback, redrawFloqueMultipliers, NULL);

        XtVaSetValues (fmDrawingArea, XmNunitType, XmPIXELS, NULL);

        XGCValues    gcv;
        gcv.foreground = WhitePixelOfScreen (XtScreen (fmDrawingArea));
        gc = XCreateGC (XtDisplay (fmDrawingArea),
            RootWindowOfScreen (XtScreen (fmDrawingArea)), GCForeground, &gcv);
        colormap = DefaultColormapOfScreen(XtScreen(fmDrawingArea));

        XSetForeground (XtDisplay (fmDrawingArea), gc,
            BlackPixelOfScreen (XtScreen (fmDrawingArea)));

        Widget form = XmCreateForm(pane, "form", NULL,0);
        str1 = XmStringCreateLocalized(str);

        n = 0;
        XtSetArg (args[n], XmNlabelString, str1);  ++n;
        XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftOffset,        5            ); n++;
        Widget label3 = XmCreateLabelGadget (form, "label", args, n);
        XtManageChild (label3);

        str1 = XmStringCreateLocalized(tmpstr);
        n = 0;
        XtSetArg (args[n], XmNlabelString, str1);  ++n;
        XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
        XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET); n++;
        XtSetArg(args[n], XmNleftWidget,    label3); n++;
        XtSetArg(args[n], XmNleftOffset,        5            ); n++;
        XtSetArg(args[n], XmNrightAttachment,   XmATTACH_FORM); n++;
        Widget label2 = XmCreateLabelGadget (form, "label", args, n);
        XtManageChild (label2);

        XtManageChild (form);

        Widget pushButton = XmCreatePushButtonGadget (pane, "OK", NULL, 0);
        XtAddCallback (pushButton, XmNactivateCallback, hidenDialogShell, dialog_shell);

        XtManageChild (pushButton);
        XtManageChild (fmDrawingArea);

        XmStringFree (str1);
        XtManageChild (pane);
    }
    XtManageChild (dialog_shell);
    delete [] str;
    delete [] tmpstr;
}


////////////////////////////////////////////////////////////////////
//
SbBool
writePickedPath (SoNode *result, const SbViewportRegion &viewport, const SbVec2s &cursorPosition)
//
////////////////////////////////////////////////////////////////////
{
    SoRayPickAction myPickAction(viewport);

    myPickAction.setPoint(cursorPosition);
    myPickAction.setRadius(4.0);

    myPickAction.apply(result);
    const SoPickedPoint *myPickedPoint =
        myPickAction.getPickedPoint();
    if (myPickedPoint == NULL)
    {
        return FALSE;
    }

    SbVec3f myPosition;
    myPosition = myPickedPoint->getPoint();
    float position[3], x, y, z;
    float * data;
    long int sIdx, bIdx;
    int size;

    myPosition.getValue(x, y, z);
    position[0]=x; position[1]=y; position[2]=z;

    lookForThePoint(position, bIdx, sIdx);
    int idix = 0;

    if(whichType != BIFURCATION &&
        (sIdx > mySolNode.totalNumPoints || sIdx < 0))
        return false;
    else if(whichType!=BIFURCATION)
    {
        data = new float[mySolNode.nar];
        if(data == NULL)
        {
            printf(" memory allocation failed!\n");
            exit(0);
        }
        size = mySolNode.nar;
        for(int ms=0; ms<mySolNode.nar; ++ms)
            data[ms]=clientData.solData[sIdx][ms];
        idix = bIdx;
    }

    if(whichType == BIFURCATION &&
        (bIdx > myBifNode.totalNumPoints || bIdx < 0))
        return false;
    else if(whichType == BIFURCATION)
    {
        data = new float[myBifNode.nar];
        if(data == NULL)
        {
            printf(" memory allocation failed!\n");
            exit(0);
        }
        size = myBifNode.nar;
        for(int ms=0; ms<myBifNode.nar; ++ms)
            data[ms]=clientData.bifData[bIdx][ms];
        idix = bIdx;
    }

    for(int ms=0; ms<clientData.numFM; ++ms)
    {
        fmData[2*ms]   = clientData.multipliers[idix][ms][0];
        fmData[2*ms+1] = clientData.multipliers[idix][ms][1];
    }

    popupFloquetMultiplierDialog(data, size);
    delete [] data;
    return TRUE;
}


///////////////////////////////////////////////////////////////////
//
// This routine is called for every mouse button event.
void
myMousePressCB(void *userData, SoEventCallback *eventCB)
//
///////////////////////////////////////////////////////////////////
{
    SoSeparator *result = (SoSeparator *) userData;
    const SoEvent *event = eventCB->getEvent();

    if (SO_MOUSE_PRESS_EVENT(event, ANY))
    {
        const SbViewportRegion &myRegion =
            eventCB->getAction()->getViewportRegion();
        writePickedPath(result, myRegion,
            event->getPosition(myRegion));
        eventCB->setHandled();
    }
}


///////////////////////////////////////////////////////////////////
//
//       Initialize the coordinate selection drop down list items
//       and the drop down label list items.
//
void
initCoordAndLableListItems()
//
///////////////////////////////////////////////////////////////////
{
    int i = 0;
    for(i=0; i<MAX_LIST; i++)
    {
        sprintf(xAxis[i], "%d", i);
        sprintf(yAxis[i], "%d", i);
        sprintf(zAxis[i], "%d", i);
    }

    int sp = 0;
    strcpy(coloringMethodList[0],"STAB"); sp++;
    strcpy(coloringMethodList[1],"PONT"); sp++;
    strcpy(coloringMethodList[2],"BRAN"); sp++;

    if(whichType == SOLUTION)
    {
        strcpy(coloringMethodList[3],"TYPE"); sp++;
        strcpy(coloringMethodList[4],"LABL"); sp++;
        strcpy(coloringMethodList[5],"COMP"); sp++; //OCT 7 added
        for(i=0; i<MAX_LIST; i++)
                sprintf(coloringMethodList[i+sp], "%d",i);
        for(i=mySolNode.nar+sp; i<mySolNode.nar+mySolNode.npar+sp; ++i)
        {
            char tmpchar[5];
            sprintf(coloringMethodList[i], "par[%d]", mySolNode.parID[i-(mySolNode.nar+sp)]);
        }
    }
    else
    {
        for(i=0; i<MAX_LIST; i++)
            sprintf(coloringMethodList[i+sp], "%d", i);
    }
    specialColorItems = sp;

    if(blOpenSolFile)
    {
        numLabels = mySolNode.numOrbits;
        myLabels[0] = 0;
        for(int j=0; j<numLabels; j++) myLabels[j+1] = mySolNode.labels[j];
    }
    else
    {
        numLabels = myBifNode.totalLabels;
        myLabels[0] = 0;
        for(int j=0; j<numLabels; j++) myLabels[j+1] = myBifNode.labels[j];
    }

    options[OPT_LEGEND] = false;
    options[OPT_BACKGROUND] = false;

    numLabels += SP_LBL_ITEMS;
    myLabels[numLabels-1] = MY_NONE;
    myLabels[numLabels-2] = MY_SPEC;
    myLabels[numLabels-3] = MY_HALF;
    for( i=0; i<numLabels; ++i)
    {
        int jmii = i + SP_LBL_ITEMS;
        sprintf(labels[jmii], "%d", myLabels[i+1]);
        switch (clientData.labelIndex[i][2])
        {
            case 1 :  
                strcat(labels[jmii]," BP");
                break;
            case 2 : 
                strcat(labels[jmii]," LP");
                break;
            case 3 :
                strcat(labels[jmii]," HB");
                break;
            case -4 :
                strcat(labels[jmii]," UZ");
                break;
            case 5 :
                strcat(labels[jmii]," LP");
                break;
            case 6 : 
                strcat(labels[jmii]," BP");
                break;
            case 7 :
                strcat(labels[jmii]," PD");
                break;
            case 8 :
                strcat(labels[jmii]," TR");
                break;
            case 9 :
                strcat(labels[jmii]," EP");
                break;
            case -9 :
                strcat(labels[jmii]," MX");
                break;
            case 0 : 
            case 4 :
            default :
                break;
        }
    }

    strcpy(labels[0],"ALL");
    strcpy(labels[1],"HALF");
    strcpy(labels[2],"SPEC");
    strcpy(labels[3],"NONE");
    numLabels--;

    if(whichType == SOLUTION)
    {
        xCoordIdxSize = dai.solXSize;
        yCoordIdxSize = dai.solYSize;
        zCoordIdxSize = dai.solZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.solX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.solY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.solZ[i];
    }
    else
    {
        xCoordIdxSize = dai.bifXSize;
        yCoordIdxSize = dai.bifYSize;
        zCoordIdxSize = dai.bifZSize;
        for(int i = 0; i<xCoordIdxSize; ++i)
            xCoordIndices[i] = dai.bifX[i];
        for(int i = 0; i<yCoordIdxSize; ++i)
            yCoordIndices[i] = dai.bifY[i];
        for(int i = 0; i<zCoordIdxSize; ++i)
            zCoordIndices[i] = dai.bifZ[i];
    }

//---------------------- Begin ---------------------------OCT 7, 04

    int half = 2;
    int iLbl = 0;
    //tmp = strtok(manyChoice, ",");
    if( lblChoice[0] == -3) // ALL
    {
        lblIndices[0] = 0;
        half = 2;
        iLbl = lblIdxSize;
    }
    else if(lblChoice[0] == -2)  // HALF
    {
        int j = 1;
        do
        {
            if(clientData.labelIndex[j][2]!= 4 || j%half == 0)
                lblIndices[iLbl++] = j;
            j++;
        } while( j < numLabels-2 );

        half *= 2;
    }
    else if(lblChoice[0] == -1) // SPEC
    {
        int j = 1;
        do
        {
            if(clientData.labelIndex[j][2] !=  TYPE_UZ    && clientData.labelIndex[j][2] != TYPE_RG )// &&
            //    clientData.labelIndex[j][2] != TYPE_EP_ODE && clientData.labelIndex[j][2] != TYPE_MX)
                lblIndices[iLbl++] = j;
            j++;
        } while( j < numLabels-2 );
        half = 2;
    }
    else if(lblChoice[0] == 0)  // NONE
    {
        lblIndices[iLbl++] = numLabels;
        half = 2;
    }
    else // Specified labels
    {
        for(int idx = 0; idx < lblIdxSize; idx++)
            lblIndices[iLbl++] = lblChoice[idx];
        half = 2;
        iLbl = lblIdxSize;
    }
    lblIdxSize = iLbl;

    optSol[0] = options[0];
    for(int i=1; i<11; ++i)
    {
        optSol[i] = options[i];
        optBif[i] = options[i];
    }
 

//----------------------- End ----------------------------

    if(!setShow3D)
    {
        for(int i=0; i<MAX_LIST; i++)
            zCoordIndices[i] = -1;
    }
}


///////////////////////////////////////////////////////////////////
//  
//       Read line colors and line pattern from the resource file.
//       each call of this function, read one line
//
void
readLineColorAndPattern(char* buffer, float *lineColor, unsigned long & linePattern)
//
///////////////////////////////////////////////////////////////////
{
    char * word ;
    for(int k=0; k<3; ++k)
    {
        word = strtok(NULL,",");
        lineColor[k]=atof(word);
    }
    word = strtok(NULL,",");
    linePattern = strtoul(word,NULL, 16);
}


///////////////////////////////////////////////////////////////////
// 
//       Read flex number of numbers.
//       each call of this function, read one line
//
void
readNData(char* buffer, float *data, int &size )
//
///////////////////////////////////////////////////////////////////
{
    char * word ; 
    int k = 0;
    if(size > 0)
    {
        for(k=0; k<size; ++k)
        {
            word = strtok(NULL,",");
            data[k]=atof(word);
        }
    }
    else
    {
        while ( (word = strtok(NULL,",") )!= NULL)
        {
            data[k++]=atof(word);
        }
        size = k;
    }
}


///////////////////////////////////////////////////////////////////
//
void
readNIntData(char* buffer, int *data, int &size )
//
///////////////////////////////////////////////////////////////////
{
    char * word ; 
    int k = 0;
    if(size > 0)
    {
        for(k=0; k<size; ++k)
        {
            word = strtok(NULL,",");
            data[k]=atoi(word);
        }
    }
    else
    {
        while ( (word = strtok(NULL,",") )!= NULL)
        {
            data[k++]=atoi(word);
        }
        size = k;
    }
}


///////////////////////////////////////////////////////////////////
//
void
readAString(char* buffer, char* aString)
//
///////////////////////////////////////////////////////////////////
{
    char * word = strtok(NULL,",");
    strcpy(aString, word);
}


///////////////////////////////////////////////////////////////////
//
void
readAHexdecimal(char* buffer, unsigned long & aHexdecimal )
//
///////////////////////////////////////////////////////////////////
{
    char * word;
    word = strtok(NULL,",");
    aHexdecimal = strtoul(word,NULL, 16);
}


///////////////////////////////////////////////////////////////////
//
int
readResourceParameters()
//
///////////////////////////////////////////////////////////////////
{
    int state = 0;

    char buffer[256];
    float lineColors[13][3];
    float aVector[3];
    unsigned long linePatterns[13];
    unsigned long stability;
    unsigned long aHex;
    char aString[256], *strTemp;
    char resource[256];

    float data;
    FILE * inFile;

    strcpy(resource, autoDir);
    strcat(resource,"/plaut04/plaut04.rc");

    inFile = fopen("plaut04.rc", "r");
    if (!inFile)
    {
        inFile = fopen(resource, "r");
        if(!inFile)
        {
            printf("Unable to open the  resource file. I will use the default values.\n");
            state = 1;
            return state;
        }
    }

    char * next;
    while ( (next=fgets(buffer, sizeof(buffer),inFile)) != NULL )
    {
        if(buffer[0] != '#')
        {
            strTemp = strtok(buffer,"=");
            strTemp = strrighttrim(strTemp);
            strTemp = strlefttrim(strTemp);

            bool blDealt = false;
            if( !blDealt )
            {
                for(int i = 0; i<NUM_SP_POINTS; ++i)
                {
                    if(strcasecmp(strTemp, typeTokenNames[i])==0)
                    {
                        readLineColorAndPattern(buffer, aVector, aHex);
                        lineColor[i].setValue( aVector );
                        linePattern[i] = aHex;
                        blDealt = true;
                        break;
                    }
                }
            }

            if( !blDealt )
            {
                for(int i = 0; i<XtNumber(graphWidgetItems); ++i)
                {
                    if(strcasecmp(strTemp, graphWidgetItems[i])==0)
                    {
                        readAString(buffer, aString);
                        char* aNewString = strrighttrim(aString);
                        aNewString = strlefttrim(aString);
                        options[i] = (strcasecmp(aNewString,"Yes")==0) ? true : false;
                        blDealt = true;
                        break;
                    }
                }
            }

            if( !blDealt )
            {
                if(strcasecmp(strTemp, "Draw Labels")==0)
                {
                    readAString(buffer, aString);
                    char* aNewString = strrighttrim(aString);
                    aNewString = strlefttrim(aString);
                    optBif[0] = (strcasecmp(aNewString,"Yes")==0) ? true : false;
                    blDealt = true;
                }
            }

            if( !blDealt )
            {
                for(int i = 0; i<XtNumber(blWidgetName); ++i)
                {
                    if(strcasecmp(strTemp, blWidgetName[i])==0)
                    {
                        readAString(buffer, aString);
                        char* aNewString = strrighttrim(aString);
                        aNewString = strlefttrim(aString);
                        setShow3D = (strcasecmp(aNewString,"Yes")==0) ? true : false;

                        blDealt = true;
                        break;
                    }
                }
            }

            if( !blDealt )
            {
                if(strcasecmp(strTemp,"Draw Scale")==0)
                {
                    readAString(buffer, aString);
                    char* aNewString = strrighttrim(aString);
                    aNewString = strlefttrim(aString);
                    blDrawTicker = (strcasecmp(aNewString,"Yes")==0) ? true : false;

                    blDealt = true;
                }
            }

            if( !blDealt )
            {
                for(int i = 0; i < XtNumber(intVariableNames) && (!blDealt); ++i)
                {
                    if(strcasecmp(strTemp, intVariableNames[i])==0)
                    {
                        readAString(buffer, aString);
                        char* aNewString = strrighttrim(aString);
                        aNewString = strlefttrim(aString);
                        switch (i)
                        {
                            case 0:
                                whichType = atoi(aString);
                                break;
                            case 1:
                                whichStyle = atoi(aString);
                                break;
                            case 2:
                                winWidth  = atoi(aString);
                                break;
                            case 3:
                                winHeight = atoi(aString);
                                break;
                            case 4:
//---------------------- Begin ---------------------------OCT 7, 04
// remove this and change the header file "gVarNames.h" also. Remove
// "Labels" from the variable name list --- intVariableNames[] . 
// the corresbonging case indices following this are also changed!
//                                lblIndices[--lblIdxSize]  = atoi(aString);
//                                lblChoice[--lblIdxSize] = atoi(aString);
//                                lblIdxSize             += 1;
//                                break;
//----------------------- End ---------------------------OCT 7, 04
                                coloringMethod = atoi(aString);
                                break;
                            case 5:
                                lineWidthScaler = atof(aString);
                                break;
                            case 6:
                                satRadius = atof(aString);
                                break;
                            case 7:
                                aniLineScaler= atof(aString);
                                break;
                            case 8:
                                MAX_SAT_SPEED = atoi(aString);
                                break;
                            case 9:
                                MIN_SAT_SPEED = atoi(aString);
                                break;
                            case 10:
                                MAX_ORBIT_SPEED = atoi(aString);
                                break;
                            case 11:
                                MIN_ORBIT_SPEED = atoi(aString);
                                break;
                            case 12:
                                whichCoord = atoi(aString);
                                break;
                            case 13:
                                bgTransparency = atof(aString);
                                break;
                            case 14:
                                numPeriodAnimated = atof(aString);
                                break;
                        }

                        blDealt = true;
                        break;
                    }
                }
            }

            if( !blDealt )
            {
                for(int i = 0; i<2; ++i)
                {
                    if(strcasecmp(strTemp, hexdecimalVarNames[i])==0)
                    {
                        readAHexdecimal(buffer, aHex);
                        stabilityLinePattern[i] = aHex;

                        blDealt = true;
                        break;
                    }
                }
            }

            if( !blDealt)
            {
                int size = 3;
                float colors[3];
                for(int i = 0; i<XtNumber(nDataVarNames); ++i)
                {
                    if(strcasecmp(strTemp, nDataVarNames[i])==0)
                    {
                        readNData(buffer, colors, size);
                        for(int j=0; j<3; ++j) envColors[i][j]=colors[j];
                        blDealt = true;
                        break;
                    }
                }
            }

            if(!blDealt && strcasecmp(strTemp, "parameter ID")==0)
            {
                int size = -1;
                int parIDs[MAX_PAR];
                readNIntData(buffer, parIDs, size);
                mySolNode.npar = size;
                blDealt = true;
                for(int is=0; is<size; ++is)
                {
                    mySolNode.parID[is] = parIDs[is];
                }
            }

//---------------------- Begin ---------------------------OCT 7, 04

            if(!blDealt && strcasecmp(strTemp, "Labels")==0)
            {
                int size = -1;
                int lblIdx[MAX_LABEL];
                readNIntData(buffer, lblIdx, size);
                lblIdxSize = size;
                blDealt = true;
                for(int is=0; is<size; ++is)
                {
                    lblChoice[is] = lblIdx[is];
                }
            }

//----------------------- End ---------------------------OCT 7, 04

            if(!blDealt)
            {
                for(int i = 0; i < XtNumber(axesNames) && (!blDealt); ++i)
                {
                    if(strcasecmp(strTemp, axesNames[i]) == 0)
                    {
                        int size = -1;
                        int pars[MAX_PAR];
                        readNIntData(buffer, pars, size);
                        mySolNode.npar = size;
                        blDealt = true;
                        switch ( i )
                        {
                            case 0:
                                dai.solXSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.solX[is] = pars[is];
                                break;
                            case 1:
                                dai.solYSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.solY[is] = pars[is];
                                break;
                            case 2:
                                dai.solZSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.solZ[is] = pars[is];
                                break;
                            case 3:
                                dai.bifXSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.bifX[is] = pars[is];
                                break;
                            case 4:
                                dai.bifYSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.bifY[is] = pars[is];
                                break;
                            case 5:
                                dai.bifZSize = size;
                                for(int is=0; is<size; ++is)
                                    dai.bifZ[is] = pars[is];
                                break;
                        }
                    }
                }
            }
        }
    }                           
    setShow3DSol = setShow3D;
    setShow3DBif = setShow3D;  
    fclose(inFile);
    return state;
}


/////////////////////////////////////////////////////////////////////
//
void
initTempVariables()
//
/////////////////////////////////////////////////////////////////////
{
    graphWidgetToggleSet = 0;
    for(int i = 0; i<XtNumber (graphWidgetItems); ++i)
    {
        if(options[i]) graphWidgetToggleSet |= (1 << i);
        optionsOld[i] = options[i];
        optionsTemp[i] = options[i];
    }

    graphWidgetToggleSetTemp = graphWidgetToggleSet;
    graphWidgetToggleSetOld  = graphWidgetToggleSet;

    whichTypeTemp = whichType;
    whichTypeOld = whichType;

    whichStyleTemp = whichStyle;
    whichStyleOld  = whichStyle;

    for(int i=0; i<NUM_SP_POINTS; ++i)
    {
        linePatternTemp[i] = linePattern[i];
        linePatternOld[i]  = linePattern[i];
        for(int j=0; j<3; ++j)
            lineColorOld[i][j] = lineColor[i][j];
    }

}


/////////////////////////////////////////////////////////////////////
//
void
setVariableDefaultValues() 
//
/////////////////////////////////////////////////////////////////////
{
    winWidth  = WIN_WIDTH;
    winHeight = WIN_HEIGHT;

    lineColor[ 0][0] = 1.0;  lineColor[ 0][1] = 1.0;  lineColor[ 0][2] = 1.0;
    lineColor[ 1][0] = 1.0;  lineColor[ 1][1] = 0.0;  lineColor[ 1][2] = 0.0;
    lineColor[ 2][0] = 0.0;  lineColor[ 2][1] = 1.0;  lineColor[ 2][2] = 0.0;
    lineColor[ 3][0] = 0.0;  lineColor[ 3][1] = 0.0;  lineColor[ 3][2] = 1.0;
    lineColor[ 4][0] = 1.0;  lineColor[ 4][1] = 1.0;  lineColor[ 4][2] = 0.0;
    lineColor[ 5][0] = 0.5;  lineColor[ 5][1] = 0.5;  lineColor[ 5][2] = 0.0;
    lineColor[ 6][0] = 0.0;  lineColor[ 6][1] = 0.0;  lineColor[ 6][2] = 0.5;
    lineColor[ 7][0] = 0.0;  lineColor[ 7][1] = 0.5;  lineColor[ 7][2] = 0.5;
    lineColor[ 8][0] = 1.0;  lineColor[ 8][1] = 0.0;  lineColor[ 8][2] = 1.0;
    lineColor[ 9][0] = 0.0;  lineColor[ 9][1] = 1.0;  lineColor[ 9][2] = 1.0;
    lineColor[10][0] = 0.3;  lineColor[10][1] = 0.0;  lineColor[10][2] = 0.3;
    lineColor[11][0] = 0.6;  lineColor[11][1] = 0.0;  lineColor[11][2] = 0.6;
    lineColor[12][0] = 1.0;  lineColor[12][1] = 1.0;  lineColor[12][2] = 1.0;

    envColors[ 0][0] = 0.0;  envColors[ 0][1] = 0.0;  envColors[ 0][2] = 0.0;
    envColors[ 1][0] = 1.0;  envColors[ 1][1] = 0.0;  envColors[ 1][2] = 0.0;
    envColors[ 2][0] = 0.0;  envColors[ 2][1] = 1.0;  envColors[ 2][2] = 0.0;
    envColors[ 3][0] = 0.0;  envColors[ 3][1] = 0.0;  envColors[ 3][2] = 1.0;
    envColors[ 4][0] = 1.0;  envColors[ 4][1] = 0.0;  envColors[ 4][2] = 0.0;
    envColors[ 5][0] = 0.0;  envColors[ 5][1] = 1.0;  envColors[ 5][2] = 0.0;
    envColors[ 6][0] = 1.0;  envColors[ 6][1] = 0.0;  envColors[ 6][2] = 0.0;
    envColors[ 7][0] = 0.0;  envColors[ 7][1] = 0.0;  envColors[ 7][2] = 1.0;
    envColors[ 8][0] = 1.0;  envColors[ 8][1] = 0.0;  envColors[ 8][2] = 1.0;
    envColors[ 9][0] = 1.0;  envColors[ 9][1] = 0.0;  envColors[ 9][2] = 0.0;

    for(int i=0; i<NUM_SP_POINTS; ++i)
        linePattern[i]   = 0xffff;

    stabilityLinePattern[0]   = 0x1111;
    stabilityLinePattern[1]   = 0xffff;

    for(int i = 0; i < sizeof(options); ++i)
        options[i] = false;

    options[OPT_NORMALIZE_DATA] = true;

    whichStyle      = 0;  
    whichType       = SOLUTION; 

    lblIdxSize       = 1;

    lblIndices[0]    = 0;   

    dai.solXSize = 1;
    dai.solYSize = 1;
    dai.solZSize = 1;
    dai.solX[0] = 0;
    dai.solY[0] = 1;
    dai.solZ[0] = 2;

    dai.bifXSize = 1;
    dai.bifYSize = 1;
    dai.bifZSize = 1;
    dai.bifX[0] = 0;
    dai.bifY[0] = 1;
    dai.bifZ[0] = 2;

    if(solHead != NULL)
    {
        solutionp cur = solHead;
        while(cur)
        {
            cur=cur->next;
            delete solHead;
            solHead = cur;
        }
    }

    setShow3D         = false;
    solHead           = NULL;
    animationLabel    = 0;
    orbitSpeed        = 1.0;
    satSpeed          = 0.5;
    lineWidthScaler   = 1.0;
    coloringMethod    = -1;
    satRadius         = 1.0;
    numPeriodAnimated = 1.0;

    mySolNode.npar= 1;
    mySolNode.parID[0] = 10;
    setShow3DSol = setShow3D;
    setShow3DBif = setShow3D;

    char * buf = new char[256];
    char * ad  = new char[256];
    strcpy(ad, "AUTO_DIR");
    if((buf=getenv(ad)) != NULL)
    {
       for(int il = 0; il < strlen(buf); ++il)
          autoDir[il] = buf[il];
    }
    else
    {
       autoDir[0]='.';
       autoDir[1]='\n';
    }
//    delete buf;
    delete ad;

}


////////////////////////////////////////////////////////////////////////
//
//  main - init Inventor and Xt, set up a scene graph and the main window,
//  display this window, and loop forever...
//
int
main(unsigned int argc, char *argv[])
//
////////////////////////////////////////////////////////////////////////
{
// usage:
// drawme --- using the default file with name "s.text" as input file.
// drawme mu fileName --- using 'fileName' as input, file1 must be a AUTO s file.

// NOTE
// if the s.xxx file does exist, the mass read from it will be used to substitue
// that from the command line
// even there are different. Because sometime the command line input mu may be
// not as accurate as the one read from the s.xxx file
// so the mu read from command line is not important in general. It can be used
// only if the s.xxx file does not exist or the system fails to read it.

    FILE *inFile;
    int  total, rows;

    strcpy(sFileName,"fort.8");
    strcpy(bFileName,"fort.7");
    strcpy(dFileName,"fort.9");

    if( argc > 1 )
    {
        if(strcmp(argv[1], "97")==0)
        {
            if( argc == 4)  
            {
                strcpy(sFileName,argv[argc-2]);
                strcat(sFileName,"/q.");
                strcat(sFileName,argv[argc-1]);

                strcpy(bFileName,argv[argc-2]);
                strcat(bFileName,"/p.");
                strcat(bFileName,argv[argc-1]);

                strcpy(dFileName,argv[argc-2]);
                strcat(dFileName,"/d.");
                strcat(dFileName,argv[argc-1]);

            } 
            else if( argc == 3)
            {
                strcpy(sFileName,"q.");
                strcat(sFileName,argv[argc-1]);

                strcpy(bFileName,"p.");
                strcat(bFileName,argv[argc-1]);

                strcpy(dFileName,"d.");
                strcat(dFileName,argv[argc-1]);
            }
            else if( argc == 2)
            {
                strcpy(sFileName,"fort.8");
                strcpy(bFileName,"fort.7");
                strcpy(dFileName,"fort.9");
            }
        }
        else
        {
            if( argc == 3)  
            {
                strcpy(sFileName,argv[argc-2]);
                strcat(sFileName,"/s.");
                strcat(sFileName,argv[argc-1]);

                strcpy(bFileName,argv[argc-2]);
                strcat(bFileName,"/b.");
                strcat(bFileName,argv[argc-1]);

                strcpy(dFileName,argv[argc-2]);
                strcat(dFileName,"/d.");
                strcat(dFileName,argv[argc-1]);

            } 
            else if( argc == 2)
            {
                strcpy(sFileName,"s.");
                strcat(sFileName,argv[argc-1]);

                strcpy(bFileName,"b.");
                strcat(bFileName,argv[argc-1]);

                strcpy(dFileName,"d.");
                strcat(dFileName,argv[argc-1]);
            }
        }
    }
    else if( argc == 1)
    {
        strcpy(sFileName,"fort.8");
        strcpy(bFileName,"fort.7");
        strcpy(dFileName,"fort.9");
    }
    else
    {
        printf(" usage: plaut04 [version] [path] [name]\n");
        printf(" For example:\n");
        printf("      plaut04            --- view the fort.7, fort.8 in the current directory \n");
        printf("      plaut04 H1         --- view s.H1, b.H1 in the current directory \n");
        printf("      plaut04 /home/he/myR3B/me H1    --- view s.H1, b.H1 in the /home/he/myR3B/me directory \n");
        printf("      plaut04 97 H1                   --- view AUTO 97 files: q.H1, p.H1 in the current directory \n");
        printf("      plaut04 97 /home/he/myR3B/me H1 --- view AUTO 97 files: q.H1, p.H1 in the /home/he/myR3B/me directory \n");
        exit(1) ;
    }

    Widget  mainWindow;
    mainWindow = SoXt::init(argv[0]);

    setVariableDefaultValues();

    readResourceParameters();

    readSolutionAndBifurcationData(1);

    initCoordAndLableListItems();
    initTempVariables();

    if (mainWindow != NULL)
    {
        root = new SoSeparator;

        SoSeparator * rootroot = new SoSeparator;
        rootroot->ref();

        root->ref();
        SoEventCallback *mouseEventCB = new SoEventCallback;
        rootroot->addChild(mouseEventCB);
        rootroot->addChild(root);

        SoXtRenderArea *ra = buildMainWindow(mainWindow, rootroot);

        mouseEventCB->addEventCallback(
            SoMouseButtonEvent::getClassTypeId(),
            myMousePressCB,
            ra->getSceneManager()->getSceneGraph());

        SoXt::show(mainWindow);
        SoXt::mainLoop();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
void
postDeals()
//
//////////////////////////////////////////////////////////////////////////
{

    delete [] mySolNode.time;
    delete [] mySolNode.xyzCoords;
    delete [] mySolNode.xAxisItems;
    delete [] mySolNode.yAxisItems;
    delete [] mySolNode.zAxisItems;

    delete [] myBifNode.xyzCoords;
    delete [] myBifNode.ptStability;

    delete [] clientData.multipliers;
    delete [] clientData.solMax;
    delete [] clientData.solMin;

    for(int i=0; i<mySolNode.totalNumPoints; ++i)
        delete [] clientData.solData[i];
    mySolNode.totalNumPoints  = 0;
    delete [] clientData.solData;
    for(int i=0; i<myBifNode.totalNumPoints; ++i)
        delete [] clientData.bifData[i];
    myBifNode.totalNumPoints = 0;
    delete [] clientData.bifData;

    delete solHead;
}


//////////////////////////////////////////////////////////////////////////
//
int
writePreferValuesToFile()
//
//////////////////////////////////////////////////////////////////////////
{
    int state = 0;

    char buffer[256];
    float lineColors[13][3];
    float aVector[3];
    unsigned long linePatterns[13];
    unsigned long stability;
    unsigned long aHex;
    char aString[256], *strTemp;

    float data;
    FILE * outFile;
    outFile = fopen("plaut04.rc.out", "w");

    if (!outFile)
    {
        printf("Unable to open the  resource file. I will use the default values.\n");
        state = 1;
        return state;
    }

    fprintf(outFile,"#version 0.0\n\n");
    fprintf(outFile,"# Line colors are represented by RGB values from 0 to 1.0.\n");
    fprintf(outFile,"# DEFAULT color is also used when animationLabel == 0, i.e.,\n");
    fprintf(outFile,"# when showing all solutions and highlighting the solutions.\n");
    fprintf(outFile,"#  Type of point     RED  GREEN  BLUE\n");

    for(int i = 0; i<NUM_SP_POINTS; ++i)
    {
        fprintf(outFile,"%-15.15s    = %3.1f,   %3.1f,   %3.1f, 0x%x\n",typeTokenNames[i],
            lineColor[i][0], lineColor[i][1],lineColor[i][2],linePattern[i]);
    }

    fprintf(outFile, "\n# Initialize the line pattern for showing stability:\n");
    for(int i = 0; i<2; ++i)
    {
        fprintf(outFile,"%-20.25s = 0x%x\n", hexdecimalVarNames[i], stabilityLinePattern[i]);
    }

    fprintf(outFile, "\n#  Initialize the default options\n");
    for(int i = 0; i<XtNumber(graphWidgetItems); ++i)
    {
        fprintf(outFile, "%-15.15s = ",graphWidgetItems[i]);
        options[i] ? fprintf(outFile, " Yes\n") : fprintf(outFile, " No\n");
    }
    fprintf(outFile, "Draw Labels     =");
    optBif[0] ? fprintf(outFile, " Yes\n") : fprintf(outFile, " No\n");


    for(int i = 0; i < XtNumber(intVariableNames); ++i)
    {
        switch (i)
        {
            case 0:
                // OK
                fprintf(outFile, "\n# Initialize the default graph type: \n"); 
                fprintf(outFile, "#  0 --- Solution (fort.8)\n");
                fprintf(outFile, "#  1 --- Bifurcation (fort.7)\n");
                fprintf(outFile, "%-25.25s  =  %i\n", intVariableNames[i],whichType);
                break;
            case 1:
//OK
                fprintf(outFile, "\n# Initialize the default graph style:\n");
                fprintf(outFile, "#  0 --- LINES,  1 --- TUBES, 2 ---- SURFACE \n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n",whichStyle);
                break;
            case 2:
                fprintf(outFile, "\n# Set the window width and height:\n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n",winWidth);
                break;
            case 3:
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n",winHeight);
                break;
            case 20:
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", lblIndices[0]);
                break;
            case 4:
                fprintf(outFile, "\n# Set coloring method.\n");
                fprintf(outFile, "#   -6 --- STABILITY\n");
                fprintf(outFile, "#   -5 --- POINT\n");
                fprintf(outFile, "#   -4 --- BRANCH\n");
                fprintf(outFile, "#   -3 --- TYPE\n");
                fprintf(outFile, "#   -2 --- LABEL\n");
                fprintf(outFile, "#   -1 --- COMPONENT\n");
                fprintf(outFile, "# Otherwise, according to the data in the ith column of the solution file.\n");
                fprintf(outFile, "# It can only be set to an integer value.\n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", coloringMethod);
                break;
            case 5:
                fprintf(outFile, "\n# Line Width Scaler adjusts the thickness of curves:\n"); 
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%4.1f\n", lineWidthScaler);
                break;
            case 6:
                fprintf(outFile, "\n# Set the radius of the animation object:\n");
                fprintf(outFile, "# The normal size is 1.0.\n");
                fprintf(outFile, "# For smaller radius, use 0.xxx\n");
                fprintf(outFile, "# For bigger radius, use  X.XXX\n"); 
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%4.1f\n", satRadius);
                break;
            case 7:
                fprintf(outFile, "\n# The AniLine Thickness Scaler sets the thickness of animated solution curves:\n"); 
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%4.1f\n", aniLineScaler);
                break;
            case 8:
                fprintf(outFile, "\n# Set the maximum and minimum animation speed:\n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", MAX_SAT_SPEED);
                break;
            case 9:
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", MIN_SAT_SPEED);
                break;
            case 10:
                fprintf(outFile, "\n# Set the maximum and minimum highlighting animation speed:\n"); 
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", MAX_ORBIT_SPEED);
                break;
            case 11:
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", MIN_ORBIT_SPEED);
                break;
            case 12:
                fprintf(outFile, "\n# Initialize the default coordinate axes:\n");
                fprintf(outFile, "#  0 --- None,\n");
                fprintf(outFile, "#  1 --- at geometry center or origin,\n");
                fprintf(outFile, "#  2 --- at left and behind,\n");
                fprintf(outFile, "#  3 --- at left and ahead. \n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%i\n", whichCoord);
                break;
            case 13:
                fprintf(outFile, "\n# Backgournd pictures transparency");
                fprintf(outFile, "\n# [0.0, 1.0] \n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%4.1f\n", bgTransparency );
                break;
            case 14:
                fprintf(outFile, "\n# Set default number of periods animated\n");
                fprintf(outFile, "\n# The value should be power of 2.\n");
                fprintf(outFile, "%-25.25s = ", intVariableNames[i]);
                fprintf(outFile, "%4.1f\n\n", numPeriodAnimated);
        }
    }

    for(int i = 0; i<XtNumber(nDataVarNames); ++i)
    {
        switch ( i ){
            case 0 :
                fprintf(outFile, "# Background color:\n");
                break;
            case 1 :
                fprintf(outFile, "# X-Axis color:\n");
                break;
            case 2 :
                fprintf(outFile, "# Y-Axis color:\n");
                break;
            case 3 :
                fprintf(outFile, "# Z-Axis color:\n");
                break;
            case 4 :
                fprintf(outFile, "# Color of the animation object:\n"); 
                break;
            case 5 :
                fprintf(outFile, "# Surface color:\n");
                break;
            case 6 :
                fprintf(outFile, "# Unstable solution Color:\n");
                break;
            case 7 :
                fprintf(outFile, "# Stable solution Color:\n");
                break;
        }

        fprintf(outFile, "%-25.25s = %4.1f, %4.1f, %4.1f\n\n",nDataVarNames[i],envColors[i][0], envColors[i][1],envColors[i][2]);
    }

//----------Labels --- ?
    fprintf(outFile, "# Labeled solutions:\n");
    fprintf(outFile, "#   -3 = Show all labeled solutions\n");
    fprintf(outFile, "#   -2 = Show HALF labeled solutions\n");
    fprintf(outFile, "#   -1 = Show SPEC labeled solutions\n");
    fprintf(outFile, "#   0 = Show NONE of the solutions\n");
    fprintf(outFile, "# Otherwise, show the specified solution(s)\n");
    for(int is=0; is<lblIdxSize; ++is)
    {
        fprintf(outFile, "Labels   = %i", lblChoice[is]); 
    }
    fprintf(outFile, "\n");

//----------

    fprintf(outFile,"\n# Draw Scale:\n");
    fprintf(outFile, "Draw Scale = ");
    (blDrawTicker) ? fprintf(outFile, " YES \n") : fprintf(outFile, " NO \n");

    fprintf(outFile,"\n# Set the active AUTO parameter indices:\n"); 
    fprintf(outFile, "parameter ID  = ");
    for(int is=0; is<mySolNode.npar; ++is)
    {
        fprintf(outFile, " %i, ", mySolNode.parID[is]);
    }
    fprintf(outFile, " \n");

    fprintf(outFile,"\n# Choose 3D or 2D graph:\n"); 
    setShow3D ? fprintf(outFile, "3D = YES\n") : fprintf(outFile, "3D = No\n");

    fprintf(outFile,"\n# Set X, Y, Z axes for the solution diagram:\n");
    fprintf(outFile,"# 0 is Time for X,Y,Z.\n");
    fprintf(outFile,"%-25.25s = ", axesNames[0]);
    for(int is=0; is<dai.solXSize; ++is)
    {
        fprintf(outFile, " %i ", dai.solX[is]);
        (is != dai.solXSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fprintf(outFile,"%-25.25s = ", axesNames[1]);
    for(int is=0; is<dai.solYSize; ++is)
    {
        fprintf(outFile, " %i ", dai.solY[is]);
        (is != dai.solYSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fprintf(outFile,"%-25.25s = ", axesNames[2]);
    for(int is=0; is<dai.solZSize; ++is)
    {
        fprintf(outFile, " %i ", dai.solZ[is]);
        (is != dai.solZSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fprintf(outFile,"\n# Set X, Y, Z axes for the bifurcation diagram:\n");
    fprintf(outFile,"# 0 is Time for X,Y,Z.\n");
    fprintf(outFile,"%-25.25s = ", axesNames[3]);
    for(int is=0; is<dai.bifXSize; ++is)
    {
        fprintf(outFile, " %i ", dai.bifX[is]);
        (is != dai.bifXSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fprintf(outFile,"%-25.25s = ", axesNames[4]);
    for(int is=0; is<dai.bifYSize; ++is)
    {
        fprintf(outFile, " %i ", dai.bifY[is]);
        (is != dai.bifYSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fprintf(outFile,"%-25.25s = ", axesNames[5]);
    for(int is=0; is<dai.bifZSize; ++is)
    {
        fprintf(outFile, " %i ", dai.bifZ[is]);
        (is != dai.bifZSize-1) ? fprintf(outFile, ",") : fprintf(outFile, "\n");
    }

    fclose(outFile);
    return state;
}


//////////////////////////////////////////////////////////////////////////
//
SoSeparator *
createStarryBackground(int total, float diameter)
//
//////////////////////////////////////////////////////////////////////////
{
    static bool starsCreated = false;
    static SoSeparator * myBg = new SoSeparator;
    if(!starsCreated)
    {
        starsCreated = true;
        srand(time(NULL));
        float div = floor(diameter);
        SoSphere * star = new SoSphere;
        star->radius = 0.002*(rand()%5+1);
        float x, y, z;
        for(int i=0; i<total; ++i)
        {
            SoSeparator * aStar = new SoSeparator;
            SoTranslation *starTrans = new SoTranslation;
            x = ((rand()%200)/100.0-1)*diameter;
            y = ((rand()%200)/100.0-1)*diameter;
            z = ((rand()%200)/100.0-1)*diameter;
            starTrans->translation.setValue(x, y, z);
            aStar->addChild(starTrans);
            aStar->addChild(star);
            myBg->addChild(aStar);
        }
    }
    myBg->unrefNoDelete();
    return myBg;
}




////////////////////////////////////////////////////////////////////////
//
//        detach everything and nuke the existing scene.
//
void
deleteScene()
//
////////////////////////////////////////////////////////////////////////
{
    for (int i = root->getNumChildren(); i>0; i--)
        root->removeChild(i-1);
}


////////////////////////////////////////////////////////////////////////
//
bool 
parseFileName(char *filename,char * sFileName, char * bFileName, char * dFileName)
//
////////////////////////////////////////////////////////////////////////
{
    char * path;
    char * name;
    char myName[256];
    char myPath[256];

    path = strtok(filename, "/");
    myPath[0]='\0';
    while(path !=NULL)
    {
        strcat(myPath,"/");
        strcat(myPath, path);
        strcpy(myName, path);
        path = strtok(NULL,"/");
    }

    strcpy(sFileName,myPath);
    strcpy(bFileName,myPath);
    strcpy(dFileName,myPath);

    int j = strlen(myPath);
    int i =strlen(myName);
    sFileName[j-i]='s'; sFileName[j-i+1]='.';
    bFileName[j-i]='b'; bFileName[j-i+1]='.';
    dFileName[j-i]='d'; dFileName[j-i+1]='.';

    return true;
}


////////////////////////////////////////////////////////////////////////
//
//        Reads the given file and insert the geometry under the selection
//  node. If the node didn't have any children, the viewAll() method is
//  automatically called.
//
SbBool
readFile(char *filename)
//
////////////////////////////////////////////////////////////////////////
{
    SbBool hadNoChildren = (root->getNumChildren() == 0);

    SbString errorMessage;
    SoSeparator *sep ;

    postDeals();
    parseFileName(filename, sFileName, bFileName, dFileName);
    bool rs = readSolutionAndBifurcationData(0);
    if(!rs)
    {
        SoText2 * errMsg = new SoText2;
        errMsg->string.setValue(" Open File Error! Maybe it is not a correct AUTO data file. ");
        root->addChild(errMsg);
        return false;
    }
    initCoordAndLableListItems();
    setListValue();
    initTempVariables();

    updateScene();

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////
//
static FILE *
convertToInventor(const char *filename)
//
////////////////////////////////////////////////////////////////////////
{
#define BUFSIZE 512
#define destinationFileType "Inventor2.1File"

    char routeprintCmd[BUFSIZE];
    char conversionCmd[BUFSIZE];
    FILE *pipeFile;
    FILE *ivDataPipe = NULL;

    sprintf(routeprintCmd, "/usr/sbin/routeprint -d %s %s 2> /dev/null",
        destinationFileType, filename);

    if (NULL != (pipeFile = popen(routeprintCmd, "r")))
    {
        (void) fgets(conversionCmd, BUFSIZE, pipeFile);
        if (0 != pclose(pipeFile))
        {
            return ivDataPipe;
        }
        ivDataPipe = popen(conversionCmd, "r");
    }

    return ivDataPipe;
}


/////////////////////////////////////////////////////////////////////////////
//
// Read all objects from the given file and return under one separator.
//
SoSeparator *
MyFileRead(const char *filename, SbString &errorMessage)
//
/////////////////////////////////////////////////////////////////////////////
{
    SoInput in;
    SbBool needToClosePipe = FALSE;
    FILE *ivDataPipe;

    if (0 != access(filename, R_OK))
    {
        errorMessage = "Error opening the file\n\"";
        errorMessage += filename;
        errorMessage += "\".";
        errorMessage += "\n\nThe file is unreadable or does not exist.";
        errorMessage += "\nYou may not have read permission on this file.";
        return NULL;
    }

    if (! in.openFile(filename, TRUE))
    {
        errorMessage = "Error opening the file\n\"";
        errorMessage += filename;
        errorMessage += "\".";
        errorMessage += "\n\nInventor was unable to read that file.";
        errorMessage += "\nYou may not have permission to read this file.";
        return NULL;
    }


    if (! in.isValidFile() )
    {
        if ((ivDataPipe = convertToInventor(filename)) != NULL)
        {
            in.setFilePointer(ivDataPipe);
            needToClosePipe = TRUE;
        }
        else
        {
            errorMessage = "Unable to read the file\n\"";
            errorMessage += filename;
            errorMessage += "\".";
            errorMessage += "\n\nInventor was unable to read that file, and";
            errorMessage += "\nthat file could not be translated to Inventor format.\n\n";
            errorMessage += "Are you sure that this is a supported filetype?";
            errorMessage += "\nYou may not have the proper translators installed,";
            errorMessage += "\nor the desktop filetype database may not be built.";
            return NULL;
        }
    }


    SoSeparator *sep = SoDB::readAll(&in);

    if (sep == NULL)
    {
        errorMessage = "Inventor encountered an error while reading\n\"";
        errorMessage += filename;
        errorMessage += "\".";
        errorMessage += "\n\nThis may not be a valid Inventor file.";
        return NULL;
    }

    in.closeFile();
    if (TRUE == needToClosePipe)
    {
        fclose(ivDataPipe);
    }
    return sep;
}