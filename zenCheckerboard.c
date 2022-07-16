#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

#include <clib/alib_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/timer_protos.h>
#include <clib/dos_protos.h>
#include <clib/asl_protos.h>


#include <intuition/intuitionbase.h>

#include <graphics/gfxbase.h>
#include <devices/timer.h>

#define __NOLIBBASE__
struct Library *CyberGfxBase=NULL;
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>

#define VERSION	"1.0"


typedef struct{
#if 0
	struct GfxBase* gfxBase;
	struct IntuitionBase *intuitionBase;
	struct DosLibrary *dosLib;
	struct AslBase *aslBase;
#endif
	struct CyberGfxBase *cgxBase;

	struct Screen *screen;
	struct Window *window;

	struct ScreenBuffer *scrBuff;
	struct BitMap *scrBm;

	int screenIsCgx;

	ULONG displayID;
	int width;
	int height;
	int depth;

}MainCtx_st,*MainCtx;


static int closeLibs(MainCtx ctx)
{

	if(ctx->cgxBase)
	{
		CloseLibrary((struct Library*)ctx->cgxBase);
		ctx->cgxBase=NULL;
	}
	
#if 0
	if(ctx->aslBase)
	{
		CloseLibrary((struct Library*)ctx->aslBase);
		ctx->aslBase=NULL;
	}

	if(ctx->dosLib)
	{
		CloseLibrary((struct Library*)ctx->dosLib);
		ctx->dosLib=NULL;
	}

	if(ctx->intuitionBase)
	{
		CloseLibrary((struct Library*)ctx->intuitionBase);
		ctx->intuitionBase=NULL;
	}
	if(ctx->gfxBase)
	{
		CloseLibrary((struct Library*)ctx->gfxBase);
		ctx->gfxBase=NULL;
	}
#endif
	return 1;
}

static int openLibs(MainCtx ctx)
{
#if 0
	ctx->gfxBase=(struct GfxBase*)OpenLibrary((unsigned char*)"graphics.library",39);
	if(!ctx->gfxBase)
	{
		printf("Failed to load graphics library!\n");
		closeLibs(ctx);
		return 0;
	}

	ctx->intuitionBase=(struct IntuitionBase*)OpenLibrary((unsigned char*)"intuition.library",39);
	if(!ctx->intuitionBase)
	{
		printf("Failed to load intuition library!\n");
		closeLibs(ctx);
		return 0;
	}

	ctx->dosLib=(struct DosLibrary*)OpenLibrary((unsigned char*)"dos.library",36);
	if(!ctx->dosLib)
	{
		printf("Failed to load dos library!\n");
		closeLibs(ctx);
		return 0;
	}

	ctx->aslBase=(struct AslBase*)OpenLibrary((unsigned char*)"asl.library",39);
	if(!ctx->aslBase)
	{
		printf("Failed to load asl library!\n");
		closeLibs(ctx);
		return 0;
	}
#endif
	ctx->cgxBase=(struct CyberGfxBase*)OpenLibrary((unsigned char*)"cybergraphics.library",39);
	if(!ctx->cgxBase)
	{
		printf(" No Cybergraphics library found : wont use cgx.\n");
	}
	CyberGfxBase=(struct Library*)ctx->cgxBase;


	return 1;
}

static int openRequester(MainCtx ctx)
{
	int ret=1;
	struct DimensionInfo query;

	struct ScreenModeRequester *scrModeReq=AllocAslRequestTags(ASL_ScreenModeRequest,
		ASLSM_TitleText,"Select screen mode.",
		ASLSM_MinDepth,8,
		ASLSM_MaxDepth,24,
		ASLSM_PropertyFlags,0,
		ASLSM_PropertyMask,0x0e,
		TAG_DONE);

	if(AslRequest(scrModeReq,NULL))
	{
		ctx->displayID=scrModeReq->sm_DisplayID;
		ctx->width=scrModeReq->sm_DisplayWidth;
		ctx->height=scrModeReq->sm_DisplayHeight;
		ctx->depth=scrModeReq->sm_DisplayDepth;		

		// because of ASL bug when choosen from keys, get dims from modeId :
		if(GetDisplayInfoData(NULL,(unsigned char*)&query,sizeof(query),DTAG_DIMS,ctx->displayID))
		{
			ctx->width=query.Nominal.MaxX-query.Nominal.MinX+1;
			ctx->height=query.Nominal.MaxY-query.Nominal.MinY+1;
		}

		printf("Choosen screen ID 0x%08x\n",(int)ctx->displayID);
		printf(" resolution : %dx%dx%dbits\n",ctx->width,ctx->height,ctx->depth);

	}
	else
	{
		printf("Aborted.\n");
		ret=0;
	}

	
	if(scrModeReq)
	{
		FreeAslRequest(scrModeReq);
	}
	return ret;
}


static struct NewScreen newscreen={
	.LeftEdge=0,
	.TopEdge=0,
	.Width=0,
	.Height=0,
	.Depth=0,
	.DetailPen=0,
	.BlockPen=1,
	.ViewModes=0, // lores no sprites
	.Type=SCREENQUIET|CUSTOMSCREEN|AUTOSCROLL,
	.Font=NULL,
	.DefaultTitle=NULL,
	.Gadgets=NULL,
	.CustomBitMap=NULL
};

static struct TagItem tagItemOpenScreen[]={
	{SA_DisplayID,0},
	{}
};

static int openScreen(MainCtx ctx)
{
	newscreen.Depth=ctx->depth;
	newscreen.Width=ctx->width;
	newscreen.Height=ctx->height;
	tagItemOpenScreen[0].ti_Data=ctx->displayID;

	ctx->screen=OpenScreenTagList(&newscreen,tagItemOpenScreen);

	if(!ctx->screen)
	{
		printf("Cannot open screen\n");
		return 0;
	}
	ctx->displayID=tagItemOpenScreen[0].ti_Data;

	return 1;
}


static int closeScreen(MainCtx ctx)
{
	int ret;
	if(!ctx->screen)
	{
		return 0;
	}
	WaitBlit();
	ret=CloseScreen(ctx->screen);
	ctx->screen=NULL;

	return ret?1:0;
}


static struct TagItem tagItemWindow[]={
	{WA_CustomScreen,	0},
	{WA_Backdrop,		-1},
	{WA_Borderless,		-1},
	{WA_Activate,		-1},
	{WA_RMBTrap,		-1},
	{WA_ReportMouse,	0},
	{WA_SizeGadget,		0},
	{WA_DepthGadget,	0},
	{WA_CloseGadget,	-1},
	{WA_DragBar,		0},
	{WA_IDCMP,			/*IDCMP_MOUSEBUTTONS|*/IDCMP_MOUSEMOVE|IDCMP_RAWKEY},
	{}
};

static int openWindow(MainCtx ctx)
{
	// get new window
	tagItemWindow[0].ti_Data=(unsigned long)ctx->screen;
	ctx->window=OpenWindowTagList(NULL,tagItemWindow);
	if(!ctx->window)
	{
		printf("Cannot open window\n");
		return 0;
	}

	return 1;
}

static int closeWindow(MainCtx ctx)
{
	if(!ctx->window)
	{
		return 0;
	}
	CloseWindow(ctx->window);

	return 1;
}

static int getCgxScreenBuffers(MainCtx ctx)
{

	printf("Screen is CGX\n");

	if(ctx->depth==8)
	{
		// load palette
		static ULONG table[]={(2l<<16)+0,
			0,0,0,
			-1,-1,-1,
			0};
		LoadRGB32(&ctx->screen->ViewPort,table);
	}

	return 1;
}

static int getAgaScreenBuffers(MainCtx ctx)
{
	printf("Screen is AGA\n");
	ctx->scrBuff=AllocScreenBuffer(ctx->screen,NULL,SB_SCREEN_BITMAP);
	if(!ctx->scrBuff)
	{
		return 0;
	}
	ctx->scrBm=ctx->scrBuff->sb_BitMap;

	// load palette
	static ULONG table[]={(2l<<16)+0,
		0,0,0,
		-1,-1,-1,
		0};
	LoadRGB32(&ctx->screen->ViewPort,table);
	return 1;
}

static int getScreenBuffers(MainCtx ctx)
{
	if(ctx->cgxBase)
	{
		BOOL isCgx=IsCyberModeID(ctx->displayID);
		if(isCgx)
		{
			ctx->screenIsCgx=1;
			return getCgxScreenBuffers(ctx);
		}
	}
	ctx->screenIsCgx=0;
	return getAgaScreenBuffers(ctx);
}


static void closeAll(MainCtx ctx)
{

	if(ctx->scrBuff)
	{
		FreeScreenBuffer(ctx->screen,ctx->scrBuff);
		ctx->scrBuff=NULL;
	}
	ctx->scrBm=NULL;

	closeWindow(ctx);
	closeScreen(ctx);
	closeLibs(ctx);
}

static void drawCheckAga(MainCtx ctx)
{
	int y;
	int bytesPerRow,almostBytesPerRow,height;
	unsigned char *pbm;

	bytesPerRow=ctx->width/8;
	almostBytesPerRow=bytesPerRow-1;
	height=ctx->height;
	
	pbm=ctx->scrBm->Planes[0];
	for(y=0;y<height;y+=2)
	{
		int x;
		*pbm++=0x55|0x80;
		for(x=1;x<bytesPerRow;x++)
		{
			*pbm++=0x55;
		}
		if(y<height)
		{
			// *pbm++=0xaa|0x80; : bit 7 already set
			for(x=0;x<almostBytesPerRow;x++)
			{
				*pbm++=0xaa;
			}
			*pbm++=0xaa|0x01;
		}
	}
	// surround screen top and bottom
	pbm=ctx->scrBm->Planes[0];
	memset(pbm,0xff,bytesPerRow); // top
	memset(pbm+bytesPerRow*(height-1),0xff,bytesPerRow); // bottom
}

static void drawCheckCgx(MainCtx ctx)
{
	int width,halfWidth,almostHalfWidth,height;
	unsigned char* CgxBaseAddress=0;
	ULONG CgxBytesPerRow=0;

	LockBitMapTags(ctx->screen->RastPort.BitMap,
		LBMI_BASEADDRESS,(ULONG)&CgxBaseAddress,
		LBMI_BYTESPERROW,(ULONG)&CgxBytesPerRow,
		TAG_DONE);

	memset(CgxBaseAddress,0,ctx->height*CgxBytesPerRow);

	width=ctx->width;
	halfWidth=width/2;
	almostHalfWidth=halfWidth-1;
	height=ctx->height;
	
	if(ctx->depth==8)
	{
		unsigned char *pbm;
		int y;
		pbm=CgxBaseAddress;
		for(y=0;y<height;y+=2)
		{
			int x;
			for(x=0;x<almostHalfWidth;x++)
			{
				*pbm=0x01; // color 1
				pbm+=2;
			}
			*pbm++=0x01; // last
			*pbm++=0x01; // right
			*pbm++=0x01; // left
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0x01; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=CgxBaseAddress;
		memset(pbm,0x01,width); // top
		memset(pbm+width*(height-1),0x01,width); // bottom
	}
	else if(ctx->depth==16)
	{
		USHORT *pbm;
		int y;
		pbm=(USHORT*)CgxBaseAddress;
		for(y=0;y<height;y+=2)
		{
			int x;
			for(x=0;x<almostHalfWidth;x++)
			{
				*pbm=0xffff; // color 1
				pbm+=2;
			}
			*pbm++=0xffff; // last
			*pbm++=0xffff; // right
			*pbm++=0xffff; // left
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(USHORT*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(USHORT)); // top
		memset((void*)pbm+width*sizeof(USHORT)*(height-1),0xff,width*sizeof(USHORT)); // bottom
	}
	else
	{
		ULONG *pbm;
		int y;
		pbm=(ULONG*)CgxBaseAddress;
		for(y=0;y<height;y+=2)
		{
			int x;
			for(x=0;x<almostHalfWidth;x++)
			{
				*pbm=0xffffffff; // color 1
				pbm+=2;
			}
			*pbm++=0xffffffff; // last
			*pbm++=0xffffffff; // right
			*pbm++=0xffffffff; // left
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffffffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(ULONG*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(ULONG)); // top
		memset((void*)pbm+width*sizeof(ULONG)*(height-1),0xff,width*sizeof(ULONG)); // bottom
	}
	UnLockBitMap(ctx->screen->RastPort.BitMap);
}

static void drawCheck(MainCtx ctx)
{
	if(ctx->screenIsCgx)
	{
		drawCheckCgx(ctx);
		return;
	}
	drawCheckAga(ctx);
}

static void drawStripesCgx(MainCtx ctx)
{
	int width,halfWidth,height;
	unsigned char* CgxBaseAddress=0;
	ULONG CgxBytesPerRow=0;

	LockBitMapTags(ctx->screen->RastPort.BitMap,
		LBMI_BASEADDRESS,(ULONG)&CgxBaseAddress,
		LBMI_BYTESPERROW,(ULONG)&CgxBytesPerRow,
		TAG_DONE);

	memset(CgxBaseAddress,0,ctx->height*CgxBytesPerRow);

	width=ctx->width;
	halfWidth=width/2;
	height=ctx->height;
	
	if(ctx->depth==8)
	{
		unsigned char *pbm;
		int y;
		pbm=CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0x01; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0x01; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=CgxBaseAddress;
		memset(pbm,0x01,width); // top
		memset(pbm+width*(height-1),0x01,width); // bottom
	}
	else if(ctx->depth==16)
	{
		USHORT *pbm;
		int y;
		pbm=(USHORT*)CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0xffff; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(USHORT*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(USHORT)); // top
		memset((void*)pbm+width*sizeof(USHORT)*(height-1),0xff,width*sizeof(USHORT)); // bottom
	}
	else
	{
		ULONG *pbm;
		int y;
		pbm=(ULONG*)CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0xffffffff; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffffffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(ULONG*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(ULONG)); // top
		memset((void*)pbm+width*sizeof(ULONG)*(height-1),0xff,width*sizeof(ULONG)); // bottom
	}
	UnLockBitMap(ctx->screen->RastPort.BitMap);
}


static void drawStripesAga(MainCtx ctx)
{
	int y;
	int bytesPerRow,height;
	unsigned char *pbm;

	bytesPerRow=ctx->width/8;
	height=ctx->height;
	
	pbm=ctx->scrBm->Planes[0];
	for(y=0;y<height;y++)
	{
		int x;
		*pbm++=0x55|0x80;
		for(x=1;x<bytesPerRow;x++)
		{
			*pbm++=0x55;
		}
	}
	// surround screen top and bottom
	pbm=ctx->scrBm->Planes[0];
	memset(pbm,0xff,bytesPerRow); // top
	memset(pbm+bytesPerRow*(height-1),0xff,bytesPerRow); // bottom
}


static void drawStripes(MainCtx ctx)
{
	if(ctx->screenIsCgx)
	{
		drawStripesCgx(ctx);
		return;
	}
	drawStripesAga(ctx);
}


static void drawLinesCgx(MainCtx ctx)
{
	int width,halfWidth,height;
	unsigned char* CgxBaseAddress=0;
	ULONG CgxBytesPerRow=0;

	LockBitMapTags(ctx->screen->RastPort.BitMap,
		LBMI_BASEADDRESS,(ULONG)&CgxBaseAddress,
		LBMI_BYTESPERROW,(ULONG)&CgxBytesPerRow,
		TAG_DONE);

	memset(CgxBaseAddress,0,ctx->height*CgxBytesPerRow);

	width=ctx->width;
	halfWidth=width/2;
	height=ctx->height;
	
	if(ctx->depth==8)
	{
		unsigned char *pbm;
		int y;
		pbm=CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0x01; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0x01; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=CgxBaseAddress;
		memset(pbm,0x01,width); // top
		memset(pbm+width*(height-1),0x01,width); // bottom
	}
	else if(ctx->depth==16)
	{
		USHORT *pbm;
		int y;
		pbm=(USHORT*)CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0xffff; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(USHORT*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(USHORT)); // top
		memset((void*)pbm+width*sizeof(USHORT)*(height-1),0xff,width*sizeof(USHORT)); // bottom
	}
	else
	{
		ULONG *pbm;
		int y;
		pbm=(ULONG*)CgxBaseAddress;
		for(y=0;y<height;y++)
		{
			int x;
			*pbm++=0xffffffff; // color 1
			for(x=0;x<halfWidth;x++)
			{
				*pbm=0xffffffff; // color 1
				pbm+=2;
			}
			pbm--;
		}
		pbm=(ULONG*)CgxBaseAddress;
		memset(pbm,0xff,width*sizeof(ULONG)); // top
		memset((void*)pbm+width*sizeof(ULONG)*(height-1),0xff,width*sizeof(ULONG)); // bottom
	}
	UnLockBitMap(ctx->screen->RastPort.BitMap);
}


static void drawLinesAga(MainCtx ctx)
{
int y;
	int bytesPerRow,height;
	unsigned char *pbm;

	bytesPerRow=ctx->width/8;
	height=ctx->height;
	
	pbm=ctx->scrBm->Planes[0];
	for(y=0;y<height;y++)
	{
		int x;
		if(y % 2 == 0) {
			*pbm++=0xff|0x80;
			for(x=1;x<(bytesPerRow-1);x++)
			{
					*pbm++=0xff;
			}
			*pbm++=0xff|0x01;
		} else {
			*pbm++=0x00|0x80;
			for(x=1;x<(bytesPerRow-1);x++)
			{
					*pbm++=0x00;
			}
			*pbm++=0x00|0x01;
		}
	}
	// surround screen top and bottom
	pbm=ctx->scrBm->Planes[0];
	memset(pbm,0xff,bytesPerRow); // top
	memset(pbm+bytesPerRow*(height-1),0xff,bytesPerRow); // bottom
}

static void drawLines(MainCtx ctx)
{
	if(ctx->screenIsCgx)
	{
		drawLinesCgx(ctx);
		return;
	}
	drawLinesAga(ctx);
}





#define CIAAPRA		0xbfe001
#define 	CIAAPRAFIR0	0x40
static int testMouseL(void)
{
	return ((*(volatile unsigned char*)CIAAPRA)&CIAAPRAFIR0)?0:1;
}

#define POTGOR 0xdff016
#define DATLY	(1<<10)
static int testMouseR(void)
{
	return ((*(volatile unsigned short*)POTGOR)&DATLY)?0:1;
}


int main(int argc,char*argv[])
{
	MainCtx_st glbCtx={};
	MainCtx ctx=&glbCtx;
	int whichDraw=0;

	printf("zenCheckerBoard v%s\n",VERSION);

	openLibs(ctx);

	if(!openRequester(ctx))
	{
		closeAll(ctx);
		return 1;
	}

	if(!openScreen(ctx))
	{
		closeAll(ctx);
		return 1;
	}

	if(!openWindow(ctx))
	{
		closeAll(ctx);
		return 1;
	}

	if(!getScreenBuffers(ctx))
	{
		closeAll(ctx);
		return 1;
	}

	drawCheck(ctx);

	// main loop : just wait
	whichDraw=0;
	
	while(!testMouseL())
	{
		// nothing
		if(testMouseR())
		{
			whichDraw--;
			if (whichDraw<0) {
				whichDraw=2;
			}
			switch(whichDraw)
			{
				case 2:
					drawLines(ctx);
					break;
				case 1:
					drawStripes(ctx);
					break;
				case 0:
				default:
					drawCheck(ctx);
			}
			while(testMouseR());
		}

	}

	closeAll(ctx);

	return 0;
}



