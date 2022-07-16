Short:        Checkerboard for video tuning
Author:       Zener (code@zener.fr)
Uploader:     Zener
Type:         util/misc
Version:      1.0
Architecture: m68k-amigaos >= 2.0.4

-----------------------------------------------------------------------------
This tool generates some checkerboards or stripes for Amiga video tuning.
It supports Aga or Cybergraphics screens.
Just run the tool from cli, it will ask what screen resolution wanted, and
 it will print checkerboard.
Press left mouse button to quit.
Press right mouse button to cycle between cherckerboard and stripes.
Sources are included, for cross-compilation using amiga-gcc from bebbo (you
 will probably have to define some vars in your setup-env script, or just
 compile from hand). You can do what you want from these sources (copyleft)
 but it is a quick and dirty tool, and may not be used as example for how
 to code on Amiga.

I quickly typed this little tool as I wanted to fine tune my OSSC from my
 Amiga 4000 with AGA connected to SCART and CyberVision64/3d connected to
 VGA input. 

-----------------------------------------------------------------------------
Tested on A4060 / OS3.9 with cyberVision64/3d.
Tested on A1200 / OS3.2 with Blizzard 1230 IV.

-----------------------------------------------------------------------------
Here you'll find the method I used to tune my OSSC (it's just my method) :

1. Set on OSSC : Post-proc. / Reverse LPF to maximum to accentuate video
    interferences.
2. Run zenCheckerboard and choose desired screen
3. In Sampling opt. / Avd. timing : 
    Center screen using synclen and backporch
    Tune samplerate until vertical interferences bars disapears (you know
     your at correct direction when number of bars decreases).
    Tune Sampling phase until checkerboard is beautiful.
4. You can choose vertical stripes (or interlace modes) using right-click.
5. Quit zenCheckerboard using left mouse button click.
6. goto step 2 to tune another screen.
7. Set correct Reverse LPF value.

-----------------------------------------------------------------------------

Version 1.0
-----------
    First Release


