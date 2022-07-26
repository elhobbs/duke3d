//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#if PLATFORM_DOS
#include <conio.h>
#endif

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "sndcards.h"
#include "fx_man.h"
#include "music.h"
#include "util_lib.h"
#include "duke3d.h"


#define LOUDESTVOLUME 64

long backflag,numenvsnds;

/*
===================
=
= SoundStartup
=
===================
*/

void SoundStartup( void )
   {
   int32 status;

   // if they chose None lets return
   if (FXDevice == NumSoundCards) return;

#ifdef __NDS__
    soundMicOff();
    soundEnable();   
#endif

   // Do special Sound Blaster, AWE32 stuff
   if (
         ( FXDevice == SoundBlaster ) ||
         ( FXDevice == Awe32 )
      )
      {
      int MaxVoices;
      int MaxBits;
      int MaxChannels;

      status = FX_SetupSoundBlaster
                  (
                  BlasterConfig, (int *)&MaxVoices, (int *)&MaxBits, (int *)&MaxChannels
                  );
      }
   else
      {
      status = FX_Ok;
      }

   if ( status == FX_Ok )
      {
      if ( eightytwofifty && numplayers > 1)
         {
         status = FX_Init( FXDevice, min( NumVoices,4 ), 1, 8, 8000 );
         }
      else
         {
         status = FX_Init( FXDevice, NumVoices, NumChannels, NumBits, MixRate );
         }
      if ( status == FX_Ok )
         {

         FX_SetVolume( FXVolume );
         if (ReverseStereo == 1)
            {
            FX_SetReverseStereo(!FX_GetReverseStereo());
            }
         }
      }
   if ( status != FX_Ok )
      {
      Error( FX_ErrorString( FX_Error ));
      }

   status = FX_SetCallBack( testcallback );

   if ( status != FX_Ok )
      {
      Error( FX_ErrorString( FX_Error ));
      }
   }

/*
===================
=
= SoundShutdown
=
===================
*/

void SoundShutdown( void )
   {
   int32 status;

   // if they chose None lets return
   if (FXDevice == NumSoundCards)
      return;

   status = FX_Shutdown();
   if ( status != FX_Ok )
      {
      Error( FX_ErrorString( FX_Error ));
      }
   }

/*
===================
=
= MusicStartup
=
===================
*/

void MusicStartup( void )
   {
   int32 status;

   // if they chose None lets return
   if ((MusicDevice == NumSoundCards) || (eightytwofifty && numplayers > 1) )
      return;

   // satisfy AWE32 and WAVEBLASTER stuff
   BlasterConfig.Midi = MidiPort;

   // Do special Sound Blaster, AWE32 stuff
   if (
         ( FXDevice == SoundBlaster ) ||
         ( FXDevice == Awe32 )
      )
      {
      int MaxVoices;
      int MaxBits;
      int MaxChannels;

      FX_SetupSoundBlaster
                  (
                  BlasterConfig, (int *)&MaxVoices, (int *)&MaxBits, (int *)&MaxChannels
                  );
      }
   status = MUSIC_Init( MusicDevice, MidiPort );

   if ( status == MUSIC_Ok )
      {
      MUSIC_SetVolume( MusicVolume );
      }
   else
   {
      puts("Couldn't find selected sound card, or, error w/ sound card itself.");

      SoundShutdown();
      uninittimer();
      uninitengine();
      CONTROL_Shutdown();
      CONFIG_WriteSetup();
      KB_Shutdown();
      uninitgroupfile();
      unlink("duke3d.tmp");
      exit(-1);
   }
}

/*
===================
=
= MusicShutdown
=
===================
*/

void MusicShutdown( void )
   {
   int32 status;

   // if they chose None lets return
   if ((MusicDevice == NumSoundCards) || (eightytwofifty && numplayers > 1) )
      return;

   status = MUSIC_Shutdown();
   if ( status != MUSIC_Ok )
      {
	//      Error( MUSIC_ErrorString( MUSIC_ErrorCode ));
      }
   }

int USRHOOKS_GetMem(char **ptr, unsigned long size )
{
   *ptr = malloc(size);

   if (*ptr == NULL)
      return(USRHOOKS_Error);

   return( USRHOOKS_Ok);

}

int USRHOOKS_FreeMem(char *ptr)
{
   free(ptr);
   return( USRHOOKS_Ok);
}

void intomenusounds(void)
{
    static const short menusnds[] =
    {
        LASERTRIP_EXPLODE,
        DUKE_GRUNT,
        DUKE_LAND_HURT,
        CHAINGUN_FIRE,
        SQUISHED,
        KICK_HIT,
        PISTOL_RICOCHET,
        PISTOL_BODYHIT,
        PISTOL_FIRE,
        SHOTGUN_FIRE,
        BOS1_WALK,
        RPG_EXPLODE,
        PIPEBOMB_BOUNCE,
        PIPEBOMB_EXPLODE,
        NITEVISION_ONOFF,
        RPG_SHOOT,
        SELECT_WEAPON
    };
    
    static int menunum = 0;
    
    sound(menusnds[menunum++]);
    menunum %= 17;
}

void playmusic(char *fn)
{
#if PLATFORM_DOS
    short      fp;
    long        l;

    if(MusicToggle == 0) return;
    if(MusicDevice == NumSoundCards) return;
    if(eightytwofifty && numplayers > 1) return;

    fp = kopen4load(fn,0);

    if(fp == -1) return;

    l = kfilelength( fp );
    if(l >= 72000)
    {
        kclose(fp);
        return;
    }

    kread( fp, MusicPtr, l);
    kclose( fp );
    MUSIC_PlaySong( MusicPtr, MUSIC_LoopSong );
#else
    if(MusicToggle == 0) return;
    if(MusicDevice == NumSoundCards) return;

    // the SDL_mixer version does more or less this same thing.  --ryan.
    PlayMusic(fn);
#endif
}
//typedef struct
//{
//   unsigned char    ucBitsPerSample;
//   unsigned char    ucChannels;
//   unsigned short   usFileFormat;
//   unsigned short   usTimeConstant;
//   long				lSamplesPerSeconds;
//   long				lTotalLength;
//} FILEINFO;

static void mix8(uint8_t *dst8, uint8_t *src8, int samples) {
    for(int i=0;i<samples;i++) {
        dst8[i] = src8[i] ^ 0x80;
    }
} 

static void mix16(uint16_t *dst16, uint16_t *src16, int samples) {
    for(int i=0;i<samples;i++) {
        dst16[i] = src16[i] ^ 0x80;
    }
} 

char loadsound(unsigned short num)
{
    long fp, l;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
    if (FXDevice == NumSoundCards) return 0;

    fp = kopen4load(sounds[num],loadfromgrouponly);
    if(fp == -1)
    {
        return 0;
    }
    l = kfilelength( fp );
    soundsiz[num] = l;
    Sound[num].lock = 200;
	int thelock = 1;
	uint16* pData;
    allocache((long *)&Sound[num].ptr,l,(unsigned char *)&Sound[num].lock);
	allocache((long *)&pData,l,(unsigned char *)&thelock);

    byte *ptr = pData;
	byte *pptrPos = Sound[num].ptr;
	
	kread( fp, pData , l);
	kclose( fp ); 

	uint8_t blockType;
	size_t blockLength;

	uint8_t freq_div;
	uint16_t codec;
	uint32_t sample_rate;
    uint32_t samples;
	uint8_t bits;
	uint8_t channels;

	if (strncmp((const char *)ptr, "Creative Voice File\x1A",10) != 0) {
		printf("not a VOC!\n");
        return 0;
	}
	ptr += 0x1a;

	do {
		blockType = *ptr;
		if (blockType != 0) {
			blockLength = ptr[1] | (ptr[2] << 8) | (ptr[3] << 16);
		}
		else {
			blockLength = 0;
		}
		ptr += 4;
		//printf("blockType: %d blockLength: %ld\n", blockType, blockLength);
		switch (blockType) {
		case 0:
			break;
		case 1:
			freq_div = ptr[0];
			codec = ptr[1];
			sample_rate = 1000000 / (256 - freq_div);
            channels = 1;
			//printf("case 1 %d %d\n", codec, sample_rate);
            switch(codec) {
                case 0:
                    bits = 8;
                    samples = blockLength-2;
                    mix8(pptrPos, &ptr[2], samples);
                    pptrPos += samples;
                    break;
                case 4:
                    bits = 16;
                    samples = blockLength-2;
                    mix16(pptrPos, &ptr[2], samples/2);
                    pptrPos += samples;
                    break;
            }
			break;
        case 2:
			//printf("case 2 %d %d\n", codec, sample_rate);
            switch(codec) {
                case 0:
                    bits = 8;
                    samples = blockLength;
                    mix8(pptrPos, &ptr[0], samples);
                    pptrPos += samples;
                    break;
                case 4:
                    bits = 16;
                    samples = blockLength;
                    mix16(pptrPos, &ptr[0], samples/2);
                    pptrPos += samples;
                    break;
            }
			break;
		case 9:
			sample_rate = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
			bits = ptr[4];
			channels = ptr[5];
			codec = ptr[6] | (ptr[7] << 8);
			//printf("case 9 %d %d %d %d\n", codec, sample_rate, bits, channels);
            switch(codec) {
                case 0:
                    samples = blockLength-12;
                    mix8(pptrPos, &ptr[12], samples);
                    pptrPos += samples;
                    break;
                case 4:
                    samples = blockLength-12;
                    mix16(pptrPos, &ptr[12], samples/2);
                    pptrPos += samples;
                    break;
            }
		default:
			break;
		}

		ptr += blockLength;
	} while (blockType != 0);


    Sound[num].bits = bits;
    Sound[num].channels = channels;
    Sound[num].speed = sample_rate;
	soundsiz[num] = (u32)ptr - (u32)pData;
	agecache();
	   
    return 1;

}

char loadsound2(unsigned short num)
{
    long   fp, l,newl;
 //  unsigned char    ucBitsPerSample;
 //  unsigned char    ucChannels;
 //  unsigned short   usFileFormat;
   unsigned short   usTimeConstant;
   byte freq_div, codec;
 //  long				lSamplesPerSeconds;
 //  long				lTotalLength;
	//byte bType;
	//signed long int lLen;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
    if (FXDevice == NumSoundCards) return 0;

    fp = kopen4load(sounds[num],loadfromgrouponly);
    if(fp == -1)
    {
        #if 0  // commented out, since game legitimately triggers it.
        sprintf(&fta_quotes[113][0],"Sound %s(#%d) not found.",sounds[num],num);
        FTA(113,&ps[myconnectindex]);
        #endif
        return 0;
    }
    l = kfilelength( fp );
    soundsiz[num] = l;
	//snddebug("size[%d].", soundsiz[num]);
    Sound[num].lock = 200;
	int thelock = 1;
	uint16* pData;// =(uint16*)(0x06020000);
	/*char* pData = (char*)malloc( soundsiz[num] );*/
	//if(pData == NULL) snddebug("out of memory");
    allocache((long *)&Sound[num].ptr,l,(unsigned char *)&Sound[num].lock);
	allocache((long *)&pData,l,(unsigned char *)&thelock);
	
    byte *pDataPos = pData;
	byte *pptrPos = Sound[num].ptr;
	
	byte bType;
    signed long int lLen;
    //kread( fp, Sound[num].ptr , l);
	kread( fp, pData , l);
	kclose( fp ); 
	unsigned char Description[20];
	memcpy(&Description, pDataPos, sizeof(Description));
	//snddebug("Description %s\n",Description);

	if (strncmp((char *)Description,"Creative Voice File\x1A",10)!=0)
	{
		printf("************NOT A VOC*********** %d\n", num);
		long size, header;
		short Compression;
		short ChannelCount;
		long SampleRate;
		short BitsPerSample;
        int i;

		//memcpy(pptrPos, pDataPos, soundsiz[num]);
        for (i = 0; i < soundsiz[num]; i++) {
            pptrPos[i] = pDataPos[i] ^ 0x80;
        }

		Sound[num].speed=8000;

		//pDataPos+=sizeof(header);
		//memcpy(&size, pDataPos, sizeof(size));pDataPos+=sizeof(size);
		//pDataPos+=sizeof(Compression);
		//pDataPos+=sizeof(ChannelCount);
		//memcpy(&SampleRate, pDataPos, sizeof(SampleRate));pDataPos+=sizeof(SampleRate);
		//Sound[num].speed=SampleRate;
		//pDataPos+=4;
		//pDataPos+=2;
		//pDataPos+=sizeof(BitsPerSample);
		//pDataPos+=(size - 16 + 1) & 0x00fe;
		//pDataPos+=sizeof(header);
		//memcpy(&size, pDataPos, sizeof(size));pDataPos+=sizeof(size);
		//memcpy(pptrPos, pDataPos, size);
		//pDataPos += size;pptrPos+=size;newl+=size;
		
	}
	else
	{
	pDataPos +=0x1A;
  do
   {
	   //snddebug("loop");
      // Read the block type
      //memcpy(&bType, pDataPos, sizeof(bType));pDataPos+=sizeof(bType);//newl+=sizeof(bType);
      bType = *pDataPos++;
      // = readBlockLen(pDataPos);
      //pDataPos += 3;
      //snddebug("bType %d",(int)bType);
      lLen = 0;
      switch( bType )
      {
         case 1:
         {
			 //snddebug("case 1");
			memcpy(&lLen, pDataPos, 3);pDataPos+=3;//newl+=3;
			//snddebug("case 1a");
            lLen -= 2;     // Remove Time Constant and File Format bytes
            //memcpy(&freq_div, pDataPos, 1);pDataPos++;//newl++;
            freq_div = *pDataPos++;
            codec = *pDataPos++;
			Sound[num].speed = 1000000 / (256 - freq_div);
			//printf("case 1 %d %d %d\n", Sound[num].speed, codec, freq_div);
            //pDataPos++;//newl++;
            // Store this sample in memory
			//snddebug("case 1b");
			//memcpy(pptrPos, pDataPos, lLen);
            int i;

            //memcpy(pptrPos, pDataPos, soundsiz[num]);
            for (i = 0; i < soundsiz[num]; i++) {
                pptrPos[i] = pDataPos[i] ^ 0x80;
            }
			//snddebug("case 1c");
            pDataPos += lLen;pptrPos+=lLen;newl+=lLen;
			 //snddebug("case 1 done");
            break;
         }
   //      case 8:
   //      {
			// //snddebug("case 8");
   //         pDataPos+=3; //newl+=3;    // Skip the length
   //         memcpy(&usTimeConstant, pDataPos, 2);pDataPos+=2;//newl+=2;
			//Sound[num].speed = 1000000 / (256-(usTimeConstant % 256));
   //         pDataPos++;//newl++;
   //         pDataPos++;//newl++;

   //         // Block of type 8 is always followed by a block of type 1
   //         memcpy(&bType, pDataPos, sizeof(bType));pDataPos+=sizeof(bType);//newl+=sizeof(bType);
   //         memcpy(&lLen, pDataPos, 3);pDataPos+=3;//newl+=3;
   //         lLen -= 2;     // Remove Time Constant and File Format bytes
   //         pDataPos+=2;//newl+=2;     // Skip T.C. and F.F.
   //         // Store this sample in memory
   //         memcpy(pptrPos, pDataPos, lLen);
   //         pDataPos += lLen;pptrPos+=lLen;newl+=lLen;
			////snddebug("case 8 done");
   //         break;
   //      }
         case 9:
         {
            memcpy(&lLen, pDataPos, 3);pDataPos+=3;//newl+=3;
            lLen -= 12;
            memcpy(&(Sound[num].speed), pDataPos, 4);//pDataPos+=12;//newl+=4;
			//printf("case 9 %d\n", Sound[num].speed);
			//Sound[num].speed=pDataPos[0] | (pDataPos[1] << 8) | (pDataPos[2] << 16) | (pDataPos[3] << 24);
            //pDataPos++;//newl++;
            //pDataPos++;//newl++;
            //pDataPos+=2;//newl+=2;
			pDataPos+=12;
            // Store this sample in memory
            //memcpy(pptrPos, pDataPos, lLen);
            int i;
            for (i = 0; i < lLen; i++) {
                pptrPos[i] = pDataPos[i] ^ 0x80;
            }
            pDataPos += lLen;pptrPos+=lLen;newl+=lLen;
			//snddebug("case 9 done");
            break;
         }
         default:
         {
			printf("case %d\n", bType);
            pDataPos += lLen;pptrPos+=lLen;//newl+=lLen;
            //do{
            //    swiWaitForVBlank();
            //} while(1);
            break;
         }
      };
   } while ( bType != 0 );
	//snddebug("loop done");
   //memset(pData,0,l);

	}
   //soundsiz[num] = newl;
	soundsiz[num] = (u32)pDataPos - (u32)pData;

	//free(pData);
	//snddebug("new size[%d].", soundsiz[num]);
	agecache();
	   
    return 1;
}

int xyzsound(short num,short i,long x,long y,long z)
{
    long sndist, cx, cy, cz, j,k;
    short pitche,pitchs,cs;
    int voice, sndang, ca, pitch;

//    if(num != 358) return 0;

    if( num >= NUM_SOUNDS ||
        FXDevice == NumSoundCards ||
        ( (soundm[num]&8) && ud.lockout ) ||
        SoundToggle == 0 ||
        Sound[num].num > 3 ||
        FX_VoiceAvailable(soundpr[num]) == 0 ||
        (ps[myconnectindex].timebeforeexit > 0 && ps[myconnectindex].timebeforeexit <= 26*3) ||
        ps[myconnectindex].gm&MODE_MENU) return -1;

    if( soundm[num]&128 )
    {
        sound(num);
        return 0;
    }

    if( soundm[num]&4 )
    {
        if(VoiceToggle==0 || (ud.multimode > 1 && PN == APLAYER && sprite[i].yvel != screenpeek && ud.coop != 1) ) return -1;

        for(j=0;j<NUM_SOUNDS;j++)
          for(k=0;k<Sound[j].num;k++)
            if( (Sound[j].num > 0) && (soundm[j]&4) )
              return -1;
    }

    cx = ps[screenpeek].oposx;
    cy = ps[screenpeek].oposy;
    cz = ps[screenpeek].oposz;
    cs = ps[screenpeek].cursectnum;
    ca = ps[screenpeek].ang+ps[screenpeek].look_ang;

    sndist = FindDistance3D((cx-x),(cy-y),(cz-z)>>4);

    if( i >= 0 && (soundm[num]&16) == 0 && PN == MUSICANDSFX && SLT < 999 && (sector[SECT].lotag&0xff) < 9 )
        sndist = divscale14(sndist,(SHT+1));

    pitchs = soundps[num];
    pitche = soundpe[num];
    cx = klabs(pitche-pitchs);

    if(cx)
    {
        if( pitchs < pitche )
             pitch = pitchs + ( rand()%cx );
        else pitch = pitche + ( rand()%cx );
    }
    else pitch = pitchs;

    sndist += soundvo[num];
    if(sndist < 0) sndist = 0;
    if( sndist && PN != MUSICANDSFX && !cansee(cx,cy,cz-(24<<8),cs,SX,SY,SZ-(24<<8),SECT) )
        sndist += sndist>>5;

    switch(num)
    {
        case PIPEBOMB_EXPLODE:
        case LASERTRIP_EXPLODE:
        case RPG_EXPLODE:
            if(sndist > (6144) )
                sndist = 6144;
            if(sector[ps[screenpeek].cursectnum].lotag == 2)
                pitch -= 1024;
            break;
        default:
            if(sector[ps[screenpeek].cursectnum].lotag == 2 && (soundm[num]&4) == 0)
                pitch = -768;
            if( sndist > 31444 && PN != MUSICANDSFX)
                return -1;
            break;
    }


    if( Sound[num].num > 0 && PN != MUSICANDSFX )
    {
        if( SoundOwner[num][0].i == i ) stopsound(num);
        else if( Sound[num].num > 1 ) stopsound(num);
        else if( badguy(&sprite[i]) && sprite[i].extra <= 0 ) stopsound(num);
    }

    if( PN == APLAYER && sprite[i].yvel == screenpeek )
    {
        sndang = 0;
        sndist = 0;
    }
    else
    {
        sndang = 2048 + ca - getangle(cx-x,cy-y);
        sndang &= 2047;
    }

    if(Sound[num].ptr == 0) { if( loadsound(num) == 0 ) return 0; }
    else
    {
       if (Sound[num].lock < 200)
          Sound[num].lock = 200;
       else Sound[num].lock++;
    }

    if( soundm[num]&16 ) sndist = 0;

    if(sndist < ((255-LOUDESTVOLUME)<<6) )
        sndist = ((255-LOUDESTVOLUME)<<6);

    if( soundm[num]&1 )
    {
        unsigned short start;

        if(Sound[num].num > 0) return -1;

        start = *(unsigned short *)(Sound[num].ptr + 0x14);
        start = BUILDSWAP_INTEL16(start);

        if(*Sound[num].ptr == 'C')
            voice = FX_PlayLoopedVOC( Sound[num].ptr, start, start + soundsiz[num],
                    pitch,sndist>>6,sndist>>6,0,soundpr[num],num);
        else
            voice = FX_PlayLoopedWAV( Sound[num].ptr, start, start + soundsiz[num],
                    pitch,sndist>>6,sndist>>6,0,soundpr[num],num);
    }
    else
    {
		//if( *Sound[num].ptr == 'C'){
  //          voice = FX_Ok;playGenericSound(Sound[ num ].ptr, soundsiz[num]);//voice = FX_PlayVOC3D( Sound[ num ].ptr,pitch,sndang>>6,sndist>>6, soundpr[num], num );
		//}
  //      else voice = FX_PlayWAV3D( Sound[ num ].ptr,pitch,sndang>>6,sndist>>6, soundpr[num], num );
		//voice = FX_Ok;
		////playGenericSound(Sound[ num ].ptr, soundsiz[num]);
		//TransferSoundData blaster = {
		//	Sound[ num ].ptr,		/* Sample address */
		//	soundsiz[num],	/* Sample length */
		//	Sound[ num ].speed,				/* Sample rate */
		//	27,				/* Volume */
		//	64,					/* panning */
		//	1 					/* format */
		// };
		//playSound(&blaster);
		voice = FX_PlayVOC3D( Sound[ num ].ptr,pitch,sndang>>6,sndist>>6, soundpr[num], num );
    }

    if ( voice > FX_Ok )
    {
        SoundOwner[num][Sound[num].num].i = i;
        SoundOwner[num][Sound[num].num].voice = voice;
        Sound[num].num++;
    }
    else Sound[num].lock--;
    return (voice);
}
extern void snddebug(const char *fmt, ...);
void sound(short num)
{
//#ifdef __NDS__
//	return;
//#endif
    short pitch,pitche,pitchs,cx;
    int voice;
    long start;

    if (FXDevice == NumSoundCards) return;
    if(SoundToggle==0) return;
    if(VoiceToggle==0 && (soundm[num]&4) ) return;
    if( (soundm[num]&8) && ud.lockout ) return;
    if(FX_VoiceAvailable(soundpr[num]) == 0) return;

	
    pitchs = soundps[num];
    pitche = soundpe[num];
    cx = klabs(pitche-pitchs);

    if(cx)
    {
        if( pitchs < pitche )
             pitch = pitchs + ( rand()%cx );
        else pitch = pitche + ( rand()%cx );
    }
    else pitch = pitchs;

    if(Sound[num].ptr == 0) { if( loadsound(num) == 0 ) return; }
    else
    {
       if (Sound[num].lock < 200)
          Sound[num].lock = 200;
       else Sound[num].lock++;
    }

    if( soundm[num]&1 )
    {
        if(*Sound[num].ptr == 'C')
        {
			snddebug("FX_PlayLoopedVOC[%d].", num);
            start = (long)*(unsigned short *)(Sound[num].ptr + 0x14);
            voice = FX_PlayLoopedVOC( Sound[num].ptr, start, start + soundsiz[num],
                    pitch,LOUDESTVOLUME,LOUDESTVOLUME,LOUDESTVOLUME,soundpr[num],num);
        }
        else
        {
			snddebug("FX_PlayLoopedWAV[%d].", num);
            start = (long)*(unsigned short *)(Sound[num].ptr + 0x14);
            voice = FX_PlayLoopedWAV( Sound[num].ptr, start, start + soundsiz[num],
                    pitch,LOUDESTVOLUME,LOUDESTVOLUME,LOUDESTVOLUME,soundpr[num],num);
        }
    }
    else
    {
    //  if(*Sound[num].ptr == 'C') {
		  //  snddebug("FX_PlayVOC3D[%d].", num);
    //        voice = FX_Ok;playGenericSound(Sound[ num ].ptr, soundsiz[num]);
		    //voice = FX_PlayVOC3D( Sound[ num ].ptr, pitch,0,255-LOUDESTVOLUME,soundpr[num], num );
    //  }
    //  else {
		  //snddebug("FX_PlayWAV3D[%d].", num);
    //        voice = FX_PlayWAV3D( Sound[ num ].ptr, pitch,0,255-LOUDESTVOLUME,soundpr[num], num );
    //  }
		//voice = FX_Ok;
		////playGenericSound(Sound[ num ].ptr, soundsiz[num]);
		//TransferSoundData blaster = {
		//	Sound[ num ].ptr,		/* Sample address */
		//	soundsiz[num],	/* Sample length */
		//	Sound[ num ].speed,				/* Sample rate */
		//	27,				/* Volume */
		//	64,					/* panning */
		//	1 					/* format */
		// };
		//playSound(&blaster);
		voice = FX_PlayVOC3D( Sound[ num ].ptr, pitch,0,255-LOUDESTVOLUME,soundpr[num], num );
    }

    if(voice > FX_Ok) return;
    Sound[num].lock--;
}

int spritesound(unsigned short num, short i)
{
    if(num >= NUM_SOUNDS) return -1;
    return xyzsound(num,i,SX,SY,SZ);
}

void stopsound(short num)
{
    if(Sound[num].num > 0)
    {
        FX_StopSound(SoundOwner[num][Sound[num].num-1].voice);
        testcallback(num);
    }
}

void stopenvsound(short num,short i)
{
    short j, k;

    if(Sound[num].num > 0)
    {
        k = Sound[num].num;
        for(j=0;j<k;j++)
           if(SoundOwner[num][j].i == i)
        {
            FX_StopSound(SoundOwner[num][j].voice);
            break;
        }
    }
}

void pan3dsound(void)
{
    long sndist, sx, sy, sz, cx, cy, cz;
    short sndang,ca,j,k,i,cs;

    numenvsnds = 0;

    if(ud.camerasprite == -1)
    {
        cx = ps[screenpeek].oposx;
        cy = ps[screenpeek].oposy;
        cz = ps[screenpeek].oposz;
        cs = ps[screenpeek].cursectnum;
        ca = ps[screenpeek].ang+ps[screenpeek].look_ang;
    }
    else
    {
        cx = sprite[ud.camerasprite].x;
        cy = sprite[ud.camerasprite].y;
        cz = sprite[ud.camerasprite].z;
        cs = sprite[ud.camerasprite].sectnum;
        ca = sprite[ud.camerasprite].ang;
    }

    for(j=0;j<NUM_SOUNDS;j++) for(k=0;k<Sound[j].num;k++)
    {
        i = SoundOwner[j][k].i;

        sx = sprite[i].x;
        sy = sprite[i].y;
        sz = sprite[i].z;

        if( PN == APLAYER && sprite[i].yvel == screenpeek)
        {
            sndang = 0;
            sndist = 0;
        }
        else
        {
            sndang = 2048 + ca - getangle(cx-sx,cy-sy);
            sndang &= 2047;
            sndist = FindDistance3D((cx-sx),(cy-sy),(cz-sz)>>4);
            if( i >= 0 && (soundm[j]&16) == 0 && PN == MUSICANDSFX && SLT < 999 && (sector[SECT].lotag&0xff) < 9 )
                sndist = divscale14(sndist,(SHT+1));
        }

        sndist += soundvo[j];
        if(sndist < 0) sndist = 0;

        if( sndist && PN != MUSICANDSFX && !cansee(cx,cy,cz-(24<<8),cs,sx,sy,sz-(24<<8),SECT) )
            sndist += sndist>>5;

        if(PN == MUSICANDSFX && SLT < 999)
            numenvsnds++;

        switch(j)
        {
            case PIPEBOMB_EXPLODE:
            case LASERTRIP_EXPLODE:
            case RPG_EXPLODE:
                if(sndist > (6144)) sndist = (6144);
                break;
            default:
                if( sndist > 31444 && PN != MUSICANDSFX)
                {
                    stopsound(j);
                    continue;
                }
        }

        if(Sound[j].ptr == 0 && loadsound(j) == 0 ) continue;
        if( soundm[j]&16 ) sndist = 0;

        if(sndist < ((255-LOUDESTVOLUME)<<6) )
            sndist = ((255-LOUDESTVOLUME)<<6);

        FX_Pan3D(SoundOwner[j][k].voice,sndang>>6,sndist>>6);
    }
}

void testcallback(unsigned long _num)
{
    long num = (long) _num;
    
    short tempi,tempj,tempk;

        if(num < 0)
        {
            if(lumplockbyte[-num] >= 200)
                lumplockbyte[-num]--;
            return;
        }

        tempk = Sound[num].num;

        if(tempk > 0)
        {
            if( (soundm[num]&16) == 0)
                for(tempj=0;tempj<tempk;tempj++)
            {
                tempi = SoundOwner[num][tempj].i;
                if(sprite[tempi].picnum == MUSICANDSFX && sector[sprite[tempi].sectnum].lotag < 3 && sprite[tempi].lotag < 999)
                {
                    hittype[tempi].temp_data[0] = 0;
                    if( (tempj + 1) < tempk )
                    {
                        SoundOwner[num][tempj].voice = SoundOwner[num][tempk-1].voice;
                        SoundOwner[num][tempj].i     = SoundOwner[num][tempk-1].i;
                    }
                    break;
                }
            }

            Sound[num].num--;
            SoundOwner[num][tempk-1].i = -1;
        }

        Sound[num].lock--;
}

void clearsoundlocks(void)
{
    long i;

    for(i=0;i<NUM_SOUNDS;i++)
        if(Sound[i].lock >= 200)
            Sound[i].lock = 199;

    for(i=0;i<11;i++)
        if(lumplockbyte[i] >= 200)
            lumplockbyte[i] = 199;
}
