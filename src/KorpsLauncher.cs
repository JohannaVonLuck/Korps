//////////////////////////////////////////////////////////////////////////////////////////
// TITLE:       KORPS GUI SYSTEM
// AUTHOR:		LANCE SIMUNEK
// DESCRIPTION: This program is a graphical user interface game launcher for the game 
//              Korps.  This is an open source project. Visit www.projectkorps.com and 
//				www.gomono.org for more information regarding that.  This program must be
//				ran under mono.  It is cross platform and can be ran on windows and Linux.
//              Following the launch of the program, you will be presented with a title
//				screen, the buttons will take you to a Single mission page, Game Options,
//				Object Viewer and Credits page.  The Korps game can be launched from this 
//				GUI, it will allow you to choose a map as well as get various other 
//				information about the models and maps.  Requires Mono and GTK# to compile.
//				Graphical User Interface was not designed using any form designers or 
//				crutches of any kind.
//////////////////////////////////////////////////////////////////////////////////////////
using System;
using System.IO;
using Gtk;
using System.Runtime.InteropServices;    // for PInvoke
using Microsoft.Win32;					 // RegistryKey

//////////////////////////////////////////////////////////////////////////////////////////
// Class:		Korps
// Description: The main class that encapsulates all functions, variables and data.
//////////////////////////////////////////////////////////////////////////////////////////
class Korps
{	
	// public variables
	Window win;
	TreeView tv;
	TextBuffer buffer;
	TextView view;
	ScrolledWindow sw;
	Notebook nb;
	VBox page3;
	VBox page5;
	
	EventBox eb1;
	EventBox eb2;
	EventBox eb3;
	EventBox eb4;
	EventBox eb5;
	EventBox eb6;
	EventBox eb7;
	string [] longtext;
	Image img2;
	Image img3;
	Image img4;
	Image img5;
	Image img6 ;
	Image img7 ;
	Image img8;
	Image img2_d;
	Image img3_d;
	Image img4_d;
	Image img5_d;
	Image img6_d;
	Image img7_d;
	Image img8_d;
	bool axis_flag;
	EventBox eb_axis;
	EventBox eb_allies;
	Image allies;
	Image allies_lit;
	Image axis;
	Image axis_lit;
	Combo combo_cov;
	Combo combo7;
	CheckButton cb2;
	Label l4;
	Combo combo;
	Combo combo3;
	CheckButton cb;
	CheckButton cb_sound;
	CheckButton cb_diameter;
	Combo combo5;
	Combo combo6;
	Combo combo_lod;
	HScale hs;
	HScale hs2;
	HScale hs3;
	TreeView tree_view;
	ob [] objects;
	Combo countries_combo;
	TextBuffer buf;
    VBox listing_vbox;
	string [] temp2;
	ScrolledWindow scr_win;
	TreeStore [] tstore;
	Image objImage;
	VBox imagebox;
	TextTag tt;
	TextTagTable ttt;
	TextTag single_tt;
	TextTagTable single_ttt;

	// variables used for settings.ini input and output
	string SCENERY_ELEMENTS = null;
	string TREE_COVERAGE = null;
	string WEATHER = null;
	string SCROLL_SPEED = null;
	string ROTATE_SPEED = null;
	string SCREEN_SIZE = null;
	string FULLSCREEN = null;
	string FILTERING = null;
	string LOD_LEVEL = null;
	string SOUND = null;
	double VOLUME = 0.0;
	string MAX_SOUNDS = null;
	string PENETRATION = null;
	string DIAMETER_FIX = null;

	// all the available game option variables
	string [] scan = new string [] { "SCENERY_ELEMENTS", "TREE_COVERAGE", "WEATHER", 
									 "DIAMETER_FIX", "LOD_LEVEL", "SCROLL_SPEED", "ROTATE_SPEED", 
									 "SCREEN_SIZE", "FULLSCREEN", "FILTERING",
									 "SOUND", "VOLUME", "MAX_SOUNDS", "PENETRATION" };
	
	// Used for sound play in windows, not implemented.
	[DllImport("winmm.dll", SetLastError=true, CallingConvention=CallingConvention.Winapi)]
	static extern bool PlaySound(
		string pszSound,
		IntPtr hMod,
		SoundFlags sf );

	// Flags for playing sounds.  For this example, we are reading 
	// the sound from a filename, so we need only specify 
	// SND_FILENAME | SND_ASYNC
	[Flags]
	public enum SoundFlags : int 
	{
		SND_SYNC = 0x0000,			// play synchronously (default) 
		SND_ASYNC = 0x0001,			// play asynchronously 
		SND_NODEFAULT = 0x0002,		// silence (!default) if sound not found 
		SND_MEMORY = 0x0004,		// pszSound points to a memory file
		SND_LOOP = 0x0008,			// loop the sound until next sndPlaySound 
		SND_NOSTOP = 0x0010,		// don't stop any currently playing sound 
		SND_NOWAIT = 0x00002000,	// don't wait if the driver is busy 
		SND_ALIAS = 0x00010000,		// name is a registry alias 
		SND_ALIAS_ID = 0x00110000,	// alias is a predefined ID
		SND_FILENAME = 0x00020000,	// name is file name 
		SND_RESOURCE = 0x00040004	// name is resource name or atom 
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	Main()
	// 
	// Description: Creates new object of type Korps
	//////////////////////////////////////////////////////////////////////////////////////////
	static void Main ()
	{
		new Korps (); 
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	Korps()
	// 
	// Description: Default constructor, initilizes application and launches appropriate 
	//              functions for initilization routines.  Finally invokes run() for the 
	//				application.
	//////////////////////////////////////////////////////////////////////////////////////////
	Korps ()
	{
		Application.Init ();
	 	  		
		SetupWindow();
		SetupTreeView();
		SetupTextView();
		SetupPacking();    	
			
		Application.Run ();
		
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupWindow()
	// 
	// Description: Sets up window dimensions and variable attributes.
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupWindow()
	{
		win = new Window ("Korps");
		
		win.SetDefaultSize (700, 403);
		win.SetSizeRequest(700, 403);

		win.DeleteEvent += new DeleteEventHandler (OnWinDelete);
		win.BorderWidth = 0;
		win.Decorated = true;
		win.Resizable = false;
		win.AllowGrow = false;
		win.AllowShrink = false;
				
		win.ModifyText(StateType.Normal, new Gdk.Color(255, 0, 0));
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupPacking()
	// 
	// Description: Does basic initilizations to the various pages in the launcher.
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupPacking()
	{
		
		VBox mainbox = new VBox(false, 0);
	
		mainbox.Show();
			
		// setup tabbed notebook
		nb = new Notebook ();
		nb.Show();
		nb.TabPos = PositionType.Bottom;
		nb.ShowTabs = false;
		

		// page0 ( MAIN ) ///////////////////////////////////////////////////////////////////////
		VBox page0 = new VBox(false, 0);
		
		page0.Show();
		HBox bottombox = new HBox();
				
		//Preload button pressed images
		
		img2_d = new Image(new Gdk.Pixbuf("./Menu/2_down.png"));
		img3_d = new Image(new Gdk.Pixbuf("./Menu/3_down.png"));
		img4_d = new Image(new Gdk.Pixbuf("./Menu/4_down.png"));
		img5_d = new Image(new Gdk.Pixbuf("./Menu/5_down.png"));
		img6_d = new Image(new Gdk.Pixbuf("./Menu/6_down.png"));
		img7_d = new Image(new Gdk.Pixbuf("./Menu/7_down.png"));
		img8_d = new Image(new Gdk.Pixbuf("./Menu/8_down.png"));
		
		// preload button up images
		Image img1 = new Image(new Gdk.Pixbuf("./Menu/1.png"));
		img2 = new Image(new Gdk.Pixbuf("./Menu/2.png"));
		img3 = new Image(new Gdk.Pixbuf("./Menu/3.png"));
		img4 = new Image(new Gdk.Pixbuf("./Menu/4.png"));
		img5 = new Image(new Gdk.Pixbuf("./Menu/5.png"));
		img6 = new Image(new Gdk.Pixbuf("./Menu/6.png"));
		img7 = new Image(new Gdk.Pixbuf("./Menu/7.png"));
		img8 = new Image(new Gdk.Pixbuf("./Menu/8.png"));
		page0.PackStart(img1, false, false, 0);
		
		//setup the various buttons
		eb1 = new EventBox ();
		bottombox.PackStart(eb1, false, false, 0);
		eb1.Add(img2);
		eb1.ButtonPressEvent += new ButtonPressEventHandler (but2_down);
		eb1.ButtonReleaseEvent += new ButtonReleaseEventHandler (campaign_click);
		
		eb2 = new EventBox ();
		bottombox.PackStart(eb2, false, false, 0);
		eb2.Add(img3);
		eb2.ButtonPressEvent += new ButtonPressEventHandler (but3_down);
		eb2.ButtonReleaseEvent += new ButtonReleaseEventHandler (singleMission_click);

		eb3 = new EventBox ();
		bottombox.PackStart(eb3, false, false, 0);
		eb3.Add(img4);
		eb3.ButtonPressEvent += new ButtonPressEventHandler (but4_down);
		eb3.ButtonReleaseEvent += new ButtonReleaseEventHandler (gameOptions_click);

		eb4 = new EventBox ();
		bottombox.PackStart(eb4, false, false, 0);
		eb4.Add(img5);
		eb4.ButtonPressEvent += new ButtonPressEventHandler (but5_down);
		eb4.ButtonReleaseEvent += new ButtonReleaseEventHandler (mapEditor_click);

		eb5 = new EventBox ();
		bottombox.PackStart(eb5, false, false, 0);
		eb5.Add(img6);
		eb5.ButtonPressEvent += new ButtonPressEventHandler (but6_down);
		eb5.ButtonReleaseEvent += new ButtonReleaseEventHandler (objectViewer_click);

		eb6 = new EventBox ();
		bottombox.PackStart(eb6, false, false, 0);
		eb6.Add(img7);
		eb6.ButtonPressEvent += new ButtonPressEventHandler (but7_down);
		eb6.ButtonReleaseEvent += new ButtonReleaseEventHandler (credits_click);

		eb7 = new EventBox ();
		bottombox.PackStart(eb7, false, false, 0);
		eb7.Add(img8);
		eb7.ButtonPressEvent += new ButtonPressEventHandler (but8_down);
		eb7.ButtonReleaseEvent += new ButtonReleaseEventHandler (exit_cb);
				
		page0.PackStart(bottombox, false, false, 0);
		
		// Set up various page layouts.

		// page1 ( Campaign ) ////////////////////////////////////////////////////////////////////
		VBox page1 = new VBox(false, 0);
		Image blank = new Image(new Gdk.Pixbuf("./Menu/blank.png"));
		Image back1 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		Image blank2 = new Image(new Gdk.Pixbuf("./Menu/blank2.png"));
		VBox layout1 = new VBox(false, 0);
		HBox h_p1 = new HBox(false, 0);
		EventBox eb_back = new EventBox();
		eb_back.Add(back1);
		
		eb_back.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		layout1.PackStart(blank, false, false, 0);
		h_p1.PackStart(eb_back, false, false, 0);
		h_p1.PackStart(blank2, false, false, 0);
		page1.PackStart(layout1, false, false, 0);
		page1.PackStart(h_p1, false, false, 0);
		

		// page2 ( Single Mission ) //////////////////////////////////////////////////////////////
		VBox page2 = new VBox(false, 0);
		VPaned splitter = new VPaned();
		splitter.ModifyBg(Gtk.StateType.Normal, new Gdk.Color(133, 133, 133));
		splitter.ModifyBg(Gtk.StateType.Active, new Gdk.Color(133, 133, 133));
		splitter.ModifyBg(Gtk.StateType.Selected, new Gdk.Color(133, 133, 133));
		splitter.ModifyBg(Gtk.StateType.Prelight, new Gdk.Color(133, 133, 133));
		sw.SetSizeRequest(30, 175);
		splitter.Pack1(sw, true, true);
		ScrolledWindow scrwin = new ScrolledWindow();
		scrwin.AddWithViewport(view);
		view.LeftMargin = 10;
		view.RightMargin = 10;

		scrwin.SetSizeRequest(30, 114);
		splitter.Pack2(scrwin, false, true);
		page2.PackStart(splitter, false, false,0);
				
		Image back2 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		Image bottom = new Image(new Gdk.Pixbuf("./Menu/bottom.png"));
		Image blankspacer = new Image(new Gdk.Pixbuf("./Menu/blankspacer.png"));
		allies = new Image(new Gdk.Pixbuf("./Menu/allies.png"));
		allies_lit = new Image(new Gdk.Pixbuf("./Menu/allies_lit.png"));
		axis = new Image(new Gdk.Pixbuf("./Menu/axis.png"));
		axis_lit = new Image(new Gdk.Pixbuf("./Menu/axis_lit.png"));
		Image launch = new Image(new Gdk.Pixbuf("./Menu/launch.png"));

		page2.PackStart(bottom, false, false, 0);
		HBox hb = new HBox();
		EventBox eb_back2 = new EventBox();
		eb_back2.Add(back2);
		eb_allies = new EventBox();
        eb_allies.Add(allies_lit);
		axis_flag = false;
		eb_allies.ButtonReleaseEvent += new ButtonReleaseEventHandler(allies_click);
		eb_axis = new EventBox();
		eb_axis.Add(axis);
		eb_axis.ButtonReleaseEvent += new ButtonReleaseEventHandler(axis_click);
		EventBox eb_launch = new EventBox();
		eb_launch.Add(launch);
		eb_launch.ButtonReleaseEvent += new ButtonReleaseEventHandler (button_click);
		eb_back2.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		
		hb.PackStart(eb_back2, false, false, 0);
		hb.PackStart(blankspacer, false, false, 0);
		hb.PackStart(eb_allies, false, false, 0);
		hb.PackStart(eb_axis, false, false, 0);
		hb.PackStart(eb_launch, false, false, 0);
		page2.PackStart(hb, false, false, 0);
		
		// page3 ( GameOptions ) ////////////////////////////////////////////////////////////////////
		SetupGameOptions();
		Image back7 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		HBox h_p3 = new HBox(false, 0);
		
		page3.PackEnd(h_p3, false, false, 0);
		EventBox eb_back7 = new EventBox();
		eb_back7.Add(back7);
		
		eb_back7.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		h_p3.PackStart(eb_back7, false, false, 0);
		
		EventBox eb_save = new EventBox();
		eb_save.Add(new Image(new Gdk.Pixbuf("./Menu/save_button.png")));
		eb_save.ButtonReleaseEvent += new ButtonReleaseEventHandler (save_settings);
		h_p3.PackStart(eb_save, false, false, 0);

		Image options_spacer = new Image(new Gdk.Pixbuf("./Menu/bottom_options.png"));
		h_p3.PackStart(options_spacer, false, false, 0);
		
		
		// page4 ( Map Editor) ////////////////////////////////////////////////////////////////////
		
		Image blank4 = new Image(new Gdk.Pixbuf("./Menu/blank.png"));
		Image back4 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		Image blank_bottom4 = new Image(new Gdk.Pixbuf("./Menu/blank2.png"));
		VBox page4 = new VBox(false, 0);
		VBox layout4 = new VBox(false, 0);
		HBox h_p4 = new HBox(false, 0);
		EventBox eb_back4 = new EventBox();
		eb_back4.Add(back4);
		
		eb_back4.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		layout4.PackStart(blank4, false, false, 0);
		h_p4.PackStart(eb_back4, false, false, 0);
		h_p4.PackStart(blank_bottom4, false, false, 0);
		page4.PackStart(layout4, false, false, 0);
		page4.PackStart(h_p4, false, false, 0);

		// page5 ( Object Viewer ) ///////////////////////////////////////////////////////////////
			
		Image back5 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		Image blank_bottom5 = new Image(new Gdk.Pixbuf("./Menu/blank2.png"));
		page5 = new VBox(false, 0);
		SetupObjectViewer();
		VBox layout5 = new VBox(false, 0);
		HBox h_p5 = new HBox(false, 0);
		EventBox eb_back5 = new EventBox();
		eb_back5.Add(back5);
		
		eb_back5.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		h_p5.PackStart(eb_back5, false, false, 0);
		h_p5.PackStart(blank_bottom5, false, false, 0);
		page5.PackEnd(h_p5, false, false, 0);

		// page6 ( Credits ) ////////////////////////////////////////////////////////////////////
		// load images for credits page
		Image credits_top = new Image(new Gdk.Pixbuf("./Menu/credits_top.png"));
		Image credits_left = new Image(new Gdk.Pixbuf("./Menu/credits_left.png"));
		Image credits_right = new Image(new Gdk.Pixbuf("./Menu/credits_right.png"));
		Image credits_bottom = new Image(new Gdk.Pixbuf("./Menu/credits_bottom.png"));
		Image back6 = new Image(new Gdk.Pixbuf("./Menu/back.png"));
		Image blank_bottom6 = new Image(new Gdk.Pixbuf("./Menu/blank2.png"));
		
		VBox page6 = new VBox(false, 0);
		VBox layout6 = new VBox(false, 0);
		layout6.PackStart(credits_top, false, false, 0);
		HBox hbawx = new HBox();
		hbawx.PackStart(credits_left, false, false, 0);
		ScrolledWindow sw_cred = new ScrolledWindow();
		
		// set scroll bar policy to be always shown on right and autmatically shown for bottom 
		sw_cred.SetPolicy(Gtk.PolicyType.Automatic, Gtk.PolicyType.Always);
		TextView credits_txt_view = new TextView();
		credits_txt_view.Editable = false;

		credits_txt_view.LeftMargin = 15;
		credits_txt_view.RightMargin = 15;
		sw_cred.AddWithViewport(credits_txt_view);

		credits_txt_view.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
		TextBuffer credits_buffer = credits_txt_view.Buffer;
		
		try 
		{
			StreamReader sr = new StreamReader("./credits.txt");
			credits_buffer.Text = sr.ReadToEnd();
			TextTagTable credits_ttt = credits_buffer.TagTable;
			TextTag credits_tt = new TextTag("sup");
			credits_tt.Family = "Courier new";
			credits_tt.Scale = 1.5;
			credits_tt.ForegroundGdk = new Gdk.Color(205, 205, 205);
			credits_ttt.Add(credits_tt);
            credits_buffer.ApplyTag(credits_tt, credits_buffer.GetIterAtOffset(0), 
									credits_buffer.GetIterAtOffset(credits_buffer.CharCount));

		}
		catch(Exception e) 
		{
			Label label = new Label("credits.txt doesn't exist no credits will be displayed");
			Label label0 = new Label(e.ToString());			
			Dialog dialog = new Dialog("Error", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			dialog.VBox.PackStart(label0, false, false, 0);

			label.Show();

			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}

		
        hbawx.PackStart(sw_cred, true, true, 0);
		hbawx.PackStart(credits_right, false, false, 0);
		layout6.PackStart(hbawx, false, false, 0);
		layout6.PackStart(credits_bottom, false, false, 0);
		HBox h_p6 = new HBox(false, 0);
		EventBox eb_back6 = new EventBox();
		eb_back6.Add(back6);
		
		eb_back6.ButtonReleaseEvent += new ButtonReleaseEventHandler(back_click2);
		
		h_p6.PackStart(eb_back6, false, false, 0);
		h_p6.PackStart(blank_bottom6, false, false, 0);
		layout6.PackStart(h_p6, false, false, 0);
		page6.PackStart(layout6, false, false, 0);

	
		string label1 = "Campaign";
		string label2 = "Single Mission";
		string label3 = "Game Options";
		string label4 = "Map Editor";
		string label5 = "Object Viewer";
		string label6 = "Credits";
		string label7 = "Exit";
		nb.AppendPage(page0, new Label (label1));
		nb.AppendPage(page1, new Label (label2));
		nb.AppendPage(page2, new Label (label3));
		nb.AppendPage(page3, new Label (label4));
		nb.AppendPage(page4, new Label (label5));
		nb.AppendPage(page5, new Label (label6));
		nb.AppendPage(page6, new Label (label7));
		
		
		mainbox.PackStart(nb, false, false, 0);
		
		win.Add(mainbox);
				
		win.ShowAll ();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	Exists()
	// 
	// Description: Parses scan array to determine if the string exists.
	//////////////////////////////////////////////////////////////////////////////////////////
	bool exists(string s)
	{		
		for(int i = 0; i < 14; i++)
		{
			// determine if scan[i] starts with the passed in string
			if(s.StartsWith(scan[i]))
			{				
				return true;
			}
		}
		
		// nope, not found 
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	set_value()
	// 
	// Description: Function is used to set all the available settings.ini variables.
	//////////////////////////////////////////////////////////////////////////////////////////
	void set_value(string property, string val)
	{
		// set all variables 
		if(property == "SCENERY_ELEMENTS")
			SCENERY_ELEMENTS = val;
		else if(property == "TREE_COVERAGE")
			TREE_COVERAGE = val;
		else if(property == "WEATHER")
			WEATHER = val;
		else if(property == "SCROLL_SPEED")
			SCROLL_SPEED = val;
		else if(property == "ROTATE_SPEED")
			ROTATE_SPEED = val;
		else if(property == "SCREEN_SIZE")
			SCREEN_SIZE = val;
		else if(property == "FULLSCREEN")
			FULLSCREEN = val;
		else if(property == "FILTERING")
			FILTERING = val;
		else if(property == "LOD_LEVEL")
			LOD_LEVEL = val;
		else if(property == "SOUND")
			SOUND = val;
		else if(property == "VOLUME")
			VOLUME = double.Parse(val);
		else if(property == "MAX_SOUNDS")
			MAX_SOUNDS = val;
		else if(property == "PENETRATION")
			PENETRATION = val;
		else if(property == "DIAMETER_FIX")
			DIAMETER_FIX = val;
		else {}
			
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupGameOptions
	// 
	// Description: Handles various routines for the game options page.  Things such as
	//              opening the settings.ini file, modifing background colors as well as text,
	//				setting up widgets, and various buttons.
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupGameOptions()
	{		
		// determine if file exists
		if(File.Exists("settings.ini"))
		{
			FileStream file = new FileStream("settings.ini", FileMode.Open, FileAccess.Read);
			// Create a new stream to read from a file
			StreamReader sr = new StreamReader(file);

			// Read contents of file into a string
			string s = sr.ReadLine();
			string property;
			string val;
			
			// while there is a line to read
			while(s!= null)
			{		
				// check if this variable is what we want
				if(exists(s))
				{						
					// strip line to get the good stuff we want to read
					property = s.Remove(s.IndexOf(' '), s.Length - s.IndexOf(' '));
					// more stripping uh huuhuhuhuhuhuh
					val = s.Remove(0, s.IndexOf('=')+1);
					val = val.Trim();					// remove trailing whitespace or endline chars
					set_value(property, val);			// set the value dude
				}
				// read next line
				s = sr.ReadLine();
			}
			// close streamreader and file 
			sr.Close();
			file.Close();
		}
		// file doesn't exit lets generate an error hehe
		else
		{

			Label label = new Label("Unable to locate settings.ini file");			
			Dialog dialog = new Dialog("Not found", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			label.Show();
				
			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}
		// lets setup the fucking notebook
		Notebook go_nb = new Notebook();
		go_nb.ModifyBg(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBg(Gtk.StateType.Active, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBg(Gtk.StateType.Insensitive, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBg(Gtk.StateType.Prelight, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBg(Gtk.StateType.Selected, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBase(Gtk.StateType.Active, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBase(Gtk.StateType.Insensitive, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBase(Gtk.StateType.Prelight, new Gdk.Color(30, 30, 30));
		go_nb.ModifyBase(Gtk.StateType.Selected, new Gdk.Color(30, 30, 30));
	
		go_nb.ModifyBg(Gtk.StateType.Active, new Gdk.Color(76, 76, 76));

		page3 = new VBox(false, 0);
		
		HBox game = new HBox(true, 0);
		HBox controls = new HBox(true, 0);
		HBox video = new HBox(true, 0);
		HBox sound = new HBox(true, 0);
		HBox realism = new HBox(false, 0);

		// game
		VBox V = new VBox(false, 5);
		VBox V2 = new VBox(false, 5);
		Label l = new Label("Scenery Elements");
		l.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		V.PackStart(l, false, false, 25);
		combo7 = new Combo(); 
		string[] list_scenery = new string[] {"NONE", "DENSE", "MODERATE", "SPARSE"};
		combo7.PopdownStrings = list_scenery;
		combo7.DisableActivate();
		combo7.ValueInList = true;
		if(SCENERY_ELEMENTS == null)
			combo7.Entry.Text = "DENSE";
		else
			combo7.Entry.Text = SCENERY_ELEMENTS;
		combo7.Entry.Editable = false;
		V2.PackStart(combo7, false, false, 20);

		Label l2 = new Label("Tree Coverage");
		l2.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		V.PackStart(l2, false, false, 10);
		combo_cov = new Combo(); 
		string[] list_tree = new string[] {"NONE", "DENSE", "MODERATE", "SPARSE"};
		combo_cov.PopdownStrings = list_scenery;
		combo_cov.DisableActivate();
		combo_cov.ValueInList = true;
		combo_cov.Entry.Editable = false;
		if(TREE_COVERAGE == null)
			combo_cov.Entry.Text = "DENSE";
		else
			combo_cov.Entry.Text = TREE_COVERAGE;
		V2.PackStart(combo_cov, false, false, 5);
		
		HBox hb_cb2 = new HBox(false, 5);
		cb2 = new CheckButton("");
		if(WEATHER == "ENABLED")
			cb2.Click();
		// setup the container for the widget to reside in 		
		hb_cb2.PackStart(cb2, false, false, 0);
		Label l_cb2 = new Label("Weather");
		hb_cb2.PackStart(l_cb2, false, false, 0);
		l_cb2.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		V.PackStart(hb_cb2, false, false, 15);
		game.PackStart(V, false, false, 10);
		game.PackStart(V2, false, false, 10);

		//controls
		VBox v8 = new VBox(false, 5);
		VBox v2_controls = new VBox();
		
		l4 = new Label("Map Scrolling Speed");
		l4.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		
		hs2 = new HScale(1, 10, 1);
		
		if(SCROLL_SPEED == null)
			hs2.Value = 5.0;
		else
			hs2.Value = double.Parse(SCROLL_SPEED);
		
		v2_controls.PackStart(hs2, false, false, 20);
		v8.PackStart(l4, false, false, 34);
		v2_controls.SetSizeRequest(200, 100);
		// setup the container for the widget to reside in 
		HBox h8 = new HBox();
		Label l3 = new Label("Map Rotating Speed");
		l3.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		h8.PackStart(l3, false, false, 20);
		
		hs3 = new HScale(1, 10, 1);

		if(ROTATE_SPEED == null)
			hs3.Value = 5.0;
		else
			hs3.Value =  double.Parse(ROTATE_SPEED);
		
		v2_controls.PackStart(hs3, false, false, 5);
		v8.PackStart(h8, false, false, 8);
		controls.PackStart(v8, false, false, 10);
		controls.PackStart(v2_controls, false, false, 0);

		//video
		VBox VBox_video = new VBox(false, 0);
		VBox VBox_video2 = new VBox( false, 0);
		Label l1 = new Label("Screen Resolution");
		l1.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		VBox_video.PackStart(l1, false, false, 25);
		string[] list = new string[] {"800x600", "1024x768", "1280x1024"};
		combo = new Combo();
		combo.PopdownStrings = list;
		combo.DisableActivate();
		if(SCREEN_SIZE == null)
			combo.Entry.Text = "800x600";
		else
			combo.Entry.Text = SCREEN_SIZE;
		combo.Entry.Editable = false;
		combo.ValueInList = true;
		VBox_video2.PackStart(combo, false, false, 20);

		//setup new label and all other widgets!
		Label l5 = new Label("Filtering");
		l5.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		VBox_video.PackStart(l5, false, false, 20);
		string[] list3 = new string[] {"None", "Linear", "Trilinear", "Anisotropic"};
		combo3 = new Combo();
		combo3.PopdownStrings = list3;
		combo3.DisableActivate();
		combo3.Entry.Editable = false;
		if(FILTERING == null)
			combo3.Entry.Text = "None";
		else
			combo3.Entry.Text = FILTERING;
		combo3.ValueInList = true;
		
		VBox_video2.PackStart(combo3, false, false, 15);
		
		//setup new label and all other widgets!
		Label l_lod = new Label("Level of Detail");
		l_lod.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		VBox_video.PackStart(l_lod, false, false, 25);
		string[] lod_strings = new string[] {"Normal", "High", "Higher", "Highest" };
		combo_lod = new Combo();
		combo_lod.PopdownStrings = lod_strings;
		combo_lod.DisableActivate();
		combo_lod.Entry.Editable = false;
		if(LOD_LEVEL == null)
			combo_lod.Entry.Text = "Normal";
		else
			combo_lod.Entry.Text = LOD_LEVEL;
		// setup the container for the widget to reside in 
		VBox_video2.PackStart(combo_lod, false, false, 15);

		HBox h9 = new HBox();
		cb = new CheckButton("");
		if(FULLSCREEN == "TRUE")
			cb.Click();

		//setup new label and all other widgets!
		Label l9 = new Label("Full Screen");
		l9.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		h9.PackStart(cb, false, false, 0);
		h9.PackStart(l9, false, false, 0);
        
		VBox_video.PackStart(h9, false, false, 5);
		
		video.PackStart(VBox_video, false, false, 10);
		video.PackStart(VBox_video2, false, false, 30);

		//sound
		VBox VBox_sound = new VBox();
		VBox VBox_sound2 = new VBox();

		Label l6 = new Label("Sound Volume     ");
		l6.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		VBox_sound.PackStart(l6, false, false, 30);
		hs = new HScale(0, 10, 1);
		
		if(VOLUME == 0.0)
			hs.Value = 0.0;
		else
			hs.Value = VOLUME;
		VBox_sound2.PackStart(hs , false, false, 20);
		//setup new label and all other widgets!
		Label l7 = new Label("Maximum Sounds");
		l7.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		VBox_sound.PackStart(l7, false, false, 12);
		string[] list5 = new string[] {"8","16","32","64"};
		combo5 = new Combo();
		combo5.PopdownStrings = list5;
		combo5.DisableActivate();
		combo5.Entry.Editable = false;
		combo5.ValueInList = true;
		if(MAX_SOUNDS == null)
			combo5.Entry.Text = "8";
		else
			combo5.Entry.Text = MAX_SOUNDS;
		VBox_sound2.PackStart(combo5, false, false, 5);

		cb_sound = new CheckButton("");
		if(SOUND == "ENABLED") 
			cb_sound.Click();
		// setup the container for the widget to reside in 
		HBox h10 = new HBox();
		Label l10 = new Label("Sound Enabled");
		l10.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205,205,205));
		h10.PackStart(cb_sound, false, false, 0);
		h10.PackStart(l10, false, false, 0);
		VBox_sound.PackStart(h10, false, false, 12);
		
		sound.PackStart(VBox_sound, false, false, 10);
		sound.PackStart(VBox_sound2, false, false, 30);
		
		//realism
		// setup the container for the widget to reside in 
		HBox h12 = new HBox(false, 5);
		Label l8 = new Label("Penetration Calculation System Basis");
		l8.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 205, 205));
		h12.PackStart(l8, false, false, 5);
		string[] list6 = new string[] {"US Test Data", "Russian Test Data"};
		combo6 = new Combo();
		combo6.PopdownStrings = list6;
		combo6.Entry.Editable = false;
		combo6.DisableActivate();
		combo6.ValueInList = true;
		
        if(PENETRATION == "US")
			combo6.Entry.Text = "US Test Data";
		else if(PENETRATION == "RU")
			combo6.Entry.Text = "Russian Test Data";
		else
			 combo6.Entry.Text = "US Test Data";


		cb_diameter = new CheckButton("");
		if(DIAMETER_FIX == "ENABLED")
			cb_diameter.Click();
	
		HBox h_diameter = new HBox();
		Label l_diameter = new Label("Slope Multiplier Diameter Fix");
		l_diameter.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205,205,205));
		h_diameter.PackStart(cb_diameter, false, false, 5);
		h_diameter.PackStart(l_diameter, false, false, 0);
		
		

		// setup the container for the widget to reside in 
		h12.PackStart(combo6, false, false, 0);
		VBox vbawx = new VBox();
		vbawx.PackStart(h12, false, false, 20);
		vbawx.PackStart(h_diameter, false, false, 15);
		realism.PackStart(vbawx, true, false, 0);
		
	
		Frame frame0 = new Frame(" Game Settings ");
		frame0.LabelWidget.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 185, 166));
		frame0.Add(game);
		Frame frame1 = new Frame(" Controls ");
		frame1.LabelWidget.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 185, 166));
		frame1.Add(controls);
		Frame frame2 = new Frame(" Video Settings ");
		frame2.LabelWidget.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 185, 166));
		frame2.Add(video);
		Frame frame3 = new Frame(" Sound Settings ");
		frame3.LabelWidget.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 185, 166));
		frame3.Add(sound);
		Frame frame4 = new Frame(" Realism Settings ");
		frame4.LabelWidget.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(205, 185, 166));
		frame4.Add(realism);
		Label game_label = new Label("Game");
		game_label.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(255, 255, 255));
		game_label.ModifyFg(Gtk.StateType.Active, new Gdk.Color(200, 200, 200));
		Label control_label = new Label("Controls");
		control_label.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(255, 255, 255));
		control_label.ModifyFg(Gtk.StateType.Active, new Gdk.Color(200, 200, 200));
		Label video_label = new Label("Video");
        video_label.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(255, 255, 255));
		video_label.ModifyFg(Gtk.StateType.Active, new Gdk.Color(200, 200, 200));
		Label sound_label = new Label("Sound");
		sound_label.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(255, 255, 255));
		sound_label.ModifyFg(Gtk.StateType.Active, new Gdk.Color(200, 200, 200));
		Label realism_label = new Label("Realism");
		realism_label.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(255, 255, 255));
		realism_label.ModifyFg(Gtk.StateType.Active, new Gdk.Color(200, 200, 200));
		go_nb.AppendPage(frame0, game_label);
		go_nb.AppendPage(frame1, control_label);
		go_nb.AppendPage(frame2, video_label);
		go_nb.AppendPage(frame3, sound_label);
		go_nb.AppendPage(frame4, realism_label);
		go_nb.SetTabLabelPacking(frame0, true, true, Gtk.PackType.Start);
		go_nb.SetTabLabelPacking(frame1, true, true, Gtk.PackType.Start);
		go_nb.SetTabLabelPacking(frame2, true, true, Gtk.PackType.Start);
		go_nb.SetTabLabelPacking(frame3, true, true, Gtk.PackType.Start);
		go_nb.SetTabLabelPacking(frame4, true, true, Gtk.PackType.Start);
		page3.PackStart(go_nb, true, true, 0);

	}

	// used for object page, each object displayed will contain various attributes:
	struct ob
	{
		public string country;
		public string name;
		public string desc;
		public string image;

		public ob(string country, string name, string desc, string image)
		{
			this.country = country;
			this.name = name;
			this.desc = desc;
			this.image = image;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupObjectViewer
	// 
	// Description: Sets up the object viewers various widgets, handles reading of images and
	//              text descriptions from file.  Modifies appropriate colors to match the
	//				korps gui game launcher theme.
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupObjectViewer()
	{		
		// Setup ListView
		try 
		{
			// Get Directory Info for Map selection
			DirectoryInfo di = new DirectoryInfo("./Menu/Objects");
			// Create an array representing the directories in the current directory.
			FileInfo[] fi = di.GetFiles(); 
			
			objects = new ob[fi.Length/2];

			StreamReader sr; 
			//longtext = new string [fi.Length];
			//string cntrys = new string [fi.Length/2];
			string temp;
			int cur = 0;

			// gather objects
			for(int i=0;i<fi.Length; i++)
			{	
				if(fi[i].Name.EndsWith(".txt"))
				{
					sr = new StreamReader("./Menu/Objects/" + fi[i].Name);
					sr.ReadLine();  // [description]
					// get country
					temp = sr.ReadLine();
					temp = temp.Remove(0, temp.IndexOf("= ", 0, 20)+1);
					temp = temp.Trim();

					objects[cur].country = temp;

					// get object name
					temp = sr.ReadLine();
					temp = temp.Remove(0, temp.IndexOf("= ", 0, 20)+1);
					temp = temp.Trim();

					objects[cur].name = temp;
					
					// get description
					temp = sr.ReadToEnd();
					temp = temp.Remove(0, temp.IndexOf("=", 0, 20)+2);
					

					objects[cur].desc = temp;

					objects[cur].image = fi[i].Name.Replace(".txt", "") + ".jpg";

					cur++;
				}
			}

			
			temp2 = new string[fi.Length];
			int size = 0;
			bool test = false;

			// parse through objects and get all countries
			for(int i = 0; i < objects.Length; i++)
			{
				if(size == 0)
				{
					temp2[0] = objects[i].country;
					size++;
				}
				else
				{
					for(int k=0; k < objects.Length;k++)
					{
						if(objects[i].country == temp2[k])
							test = true;
					}

					if(test == false)
					{
						temp2[size] = objects[i].country;
						size++;
					}
					test = false;

				}
			}

			
			tstore = new TreeStore[size];  // for each country
			// Setup TreeModels for each country
			for (int i = 0 ; i < size; i++)
			{
				tstore[i] = new TreeStore(typeof(string));
			}

			
			// iterate through entire array if country is equal
			for(int i=0;i<objects.Length; i++)
			{
				for(int j=0;j<size;j++)
				{
					if(objects[i].country == temp2[j])
					{						
						TreeIter iter = tstore[j].AppendValues(objects[i].name);
					}
				}
			}
			
			// setup a treeview widget!!!!!
			tree_view = new TreeView();
			tree_view.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
			tree_view.Model = tstore[0];
			tree_view.HeadersVisible = false;
			
			CellRendererText cell = new CellRendererText();
			cell.Foreground = "#cdcdcd";
            tree_view.AppendColumn("Object", cell, "text", 0);
			
			tree_view.Selection.Changed += new EventHandler(object_change);

		}// handle exceptions
		catch(Exception e) 
		{
			Label label = new Label("Directory 'Menu/Objects' doesn't exist (or no txt files in there)! Nothing will be displayed");
			Label label2 = new Label(e.ToString());			
			Dialog dialog = new Dialog("Error", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			dialog.VBox.PackStart(label2, false, false, 0);

			label.Show();

			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}

		
		// createa  splitter and adjust colors and set it's scroll policy now!
		HPaned splitter2 = new HPaned();
		splitter2.ModifyBg(Gtk.StateType.Normal, new Gdk.Color(133, 133, 133));
		splitter2.ModifyBg(Gtk.StateType.Active, new Gdk.Color(133, 133, 133));
		splitter2.ModifyBg(Gtk.StateType.Selected, new Gdk.Color(133, 133, 133));
		splitter2.ModifyBg(Gtk.StateType.Prelight, new Gdk.Color(133, 133, 133));
		scr_win = new ScrolledWindow();
		ScrolledWindow scr2_win = new ScrolledWindow();
		scr2_win.SetPolicy(PolicyType.Never, PolicyType.Always);
		imagebox = new VBox();
		
		try
		{
			objImage = new Image(new Gdk.Pixbuf("./Menu/Objects/" + objects[0].image));
		}
		catch(Exception e)
		{

			Label label = new Label(e.ToString());			
			Dialog dialog = new Dialog("Error", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			
			label.Show();

			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}
		// add image
		imagebox.Add(objImage);
		HBox obj_hbox = new HBox();
		VBox obj_vbox = new VBox();

		// new text view
		TextView txt_view = new TextView();
		txt_view.Editable = false;
		txt_view.LeftMargin = 10;
		txt_view.RightMargin = 10;

		txt_view.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));

		scr_win.SetSizeRequest(125, 200);
		
		buf = txt_view.Buffer;
		txt_view.WrapMode = WrapMode.Word;
		
		// set the text buffer equal to the text 
		buf.Text = objects[0].desc;
		tt = new TextTag("hello");
		tt.ForegroundGdk = new Gdk.Color(205, 205, 205);
		tt.Family = "Courier new";
		tt.Scale = 1.3;
		ttt = buf.TagTable;
		ttt.Add(tt);
		
		buf.ApplyTag(tt, buf.GetIterAtOffset(0), buf.GetIterAtOffset(buf.CharCount));
		
		// setup container 
		listing_vbox = new VBox();
		countries_combo = new Combo();
		countries_combo.PopdownStrings = temp2;
		countries_combo.Entry.Changed += new EventHandler (country_changed);
		countries_combo.WidthRequest = 10;
		listing_vbox.PackStart(countries_combo, false, false, 0);
		listing_vbox.PackStart(tree_view, true, true, 0);

		// let the scroled window know what it needs to display
		scr_win.AddWithViewport(listing_vbox);
		obj_vbox.PackStart(imagebox, false, false, 0);
		obj_vbox.PackStart(txt_view, true, true, 0);
		scr2_win.AddWithViewport(obj_vbox);
		splitter2.Add1(scr_win);
		splitter2.Add2(scr2_win);
		obj_hbox.PackStart(splitter2, true, true, 0);
		page5.PackStart(obj_hbox, true, true, 0);

	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupTextView()
	// 
	// Description: Sets up the TextView widget which is contained in the Single Missions 
	//				page.
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupTextView()
	{
		view = new Gtk.TextView ();
		buffer = view.Buffer;

		view.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
		view.Editable = false;
		view.CursorVisible = false;
		view.WrapMode = WrapMode.Word;

		single_ttt = buffer.TagTable;
		single_tt = new TextTag("hehe");
		single_tt.Family = "Courier new";
		single_tt.Scale = 1.4;
		single_tt.ForegroundGdk = new Gdk.Color(205, 205, 205);
		single_ttt.Add(single_tt);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	SetupTreeView()
	// 
	// Description: Sets up the TreeView widget for the single missions page which will list
	//				the various missions that are available, these are read in from file.
	//				TreeModel widget is also setup here.  Mission description is also read
	//				from file. Appropriate colors are modified for korps theme.
	//
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetupTreeView()
	{
		// window to hold TreeView
		sw = new ScrolledWindow (); 
		//sw.SetSizeRequest(640, 270);
		
		// Tree model of type store
		TreeStore store = new TreeStore (typeof (int), typeof (string), typeof (string), 
										 typeof (string), typeof (string), typeof (string), 
										 typeof (string));
		
		// Setup ListView
		try 
		{
			// Get Directory Info for Map selection
			DirectoryInfo di = new DirectoryInfo("./Missions");
			// Create an array representing the directories in the current directory.
			DirectoryInfo[] fi = di.GetDirectories();  
			
			StreamReader sr; 
			longtext = new string [fi.Length];
			// fill treestore with data
			for(int i=0;i<fi.Length; i++)
			{	
				sr = new StreamReader("./Missions/" + fi[i].Name + "/mission.text");
				sr.ReadLine();
				TreeIter iter = store.AppendValues (i, fi[i].Name, sr.ReadLine(), sr.ReadLine(), sr.ReadLine(), sr.ReadLine(), sr.ReadLine() ); 
				longtext[i] = sr.ReadLine();
			}

		}
		catch(Exception e) 
		{
			Label label = new Label("Directory 'Missions' doesn't exist (or mission.txt files are not there)!  No map information will be displayed.");
			Label label2 = new Label(e.ToString());			
			Dialog dialog = new Dialog("Error", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			dialog.VBox.PackStart(label2, false, false, 0);

			label.Show();

			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}

		// lets create a new tree view		
		tv = new TreeView();
		
		// let the treeview know which model it needs
		tv.Model = store;
		tv.HeadersVisible = true;
		tv.HeadersClickable = true;
		tv.ModifyBase(Gtk.StateType.Normal, new Gdk.Color(30, 30, 30));
		CellRendererText cell = new CellRendererText();
	
		cell.Foreground = "#cdcdcd";
	
		// append columns that the tree view will contain
		tv.AppendColumn ( "Mission", cell, "text", 1);
		tv.AppendColumn ( "Theatre", cell, "text", 2);		
		tv.AppendColumn ( "Date", cell, "text", 3);
		tv.AppendColumn ( "Allied Country", cell, "text", 4);
		tv.AppendColumn ( "Axis Country", cell, "text", 5);
		tv.AppendColumn ( "Info", cell, "text", 6);
		
		// all headers
		string [] headers = { "Mission", "Theatre", "Date", "Allied Country", "Axis Country", "Info" };
		
		// parse through headers and create new labels and modify their colors!
		for(int i = 0; i < 6; i++)
		{
			TreeViewColumn tvc = tv.GetColumn(i);
			Label lab = new Label(headers[i]);
			lab.ModifyFg(Gtk.StateType.Normal, new Gdk.Color(240, 235, 196));
			lab.Show();
		
			tvc.Widget = lab;
			tvc.Widget.Parent.Parent.Parent.ModifyBg(Gtk.StateType.Normal, new Gdk.Color(0, 0, 0));
		}

		sw.Add(tv);
		sw.Show();		
		
		// register event handler
		tv.Selection.Changed += new EventHandler(list_change);
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	OnWinDelete
	// 
	// Description: Invokes quit() for application.
	//////////////////////////////////////////////////////////////////////////////////////////
	void OnWinDelete (object obj, DeleteEventArgs args)
	{
		Application.Quit ();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	button_click()
	// 
	// Description: This function dispatches events for when the TreeView various rows are
	//				clicked.  Things such as proper text description of the map will be displayed
	//				in the lower TextView widget.  Function also sets flags for axis and ally
	//				choice.
	//////////////////////////////////////////////////////////////////////////////////////////
	void button_click (object o, ButtonReleaseEventArgs args)
	{
		TreeIter iter;
		TreeModel model;
 		
		// if a row is selected in the treeview
		if (tv.Selection.GetSelected (out model, out iter))
		{
			try
			{
		
				System.Diagnostics.Process proc = new System.Diagnostics.Process();
				proc.EnableRaisingEvents=true;
				
				// determine OS
				if(File.Exists("korps.ico"))
					proc.StartInfo.FileName="main";
				else
					proc.StartInfo.FileName="./main";
				
				string val = (string) model.GetValue (iter, 1);
				string country;
				string axis_alli;
					
				// check whether axis or allies was clicked
				if(axis_flag == true)
				{
					axis_alli = "AXIS";
					country = (string) model.GetValue( iter, 4);
				}
				else
				{
					axis_alli = "ALLIED";
					country = (string) model.GetValue( iter, 5);
				}
		
				string arguments = "\"Missions/" + val + "\"" + " " + axis_alli + " " +  country;
			    
				// process start arguments
				proc.StartInfo.Arguments = arguments;
				proc.Start();
				proc.WaitForExit();
			}
			catch(Exception e) 
			{
				Label label = new Label(e.ToString());			
				Dialog dialog = new Dialog("Error", win, Gtk.DialogFlags.DestroyWithParent);
				dialog.Modal = true;
				dialog.VBox.PackStart(label, false, false, 0);
				label.Show();
				
				dialog.AddButton ("Close", ResponseType.Close);
				dialog.Run ();
				dialog.Destroy ();  

			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	exit_cb()
	// 
	// Description: Handles button click event on main page's exit button.
	//////////////////////////////////////////////////////////////////////////////////////////
	// Exit via Menu event
	static void exit_cb (object obj, ButtonReleaseEventArgs args)
	{
		Application.Quit();    
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	list_change
	// 
	// Description: Handles when the list is changed event.  
	//////////////////////////////////////////////////////////////////////////////////////////
	void list_change (object obj, EventArgs args)
	{
		
		TreeIter      iter;
		TreeModel     model;
		
		// handle the changing of the list
		if (((TreeSelection)obj).GetSelected (out model, out iter))
		{
			int val = (int)model.GetValue (iter, 0);
			buffer.Text = longtext[val];

			buffer.ApplyTag(single_tt, buffer.GetIterAtOffset(0), buffer.GetIterAtOffset(buffer.CharCount));
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	campaign_click()
	// 
	// Description: Campaign button is clicked, this function handles image swap and page
	//				change.
	//////////////////////////////////////////////////////////////////////////////////////////
	void campaign_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 1;
		eb1.Remove(img2_d);
		eb1.Add(img2);
		img2.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	singleMission_click
	// 
	// Description: Single Mission button is clicked, this function handles image swap and page
	//				change.
	//////////////////////////////////////////////////////////////////////////////////////////
	void singleMission_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 2;
		eb2.Remove(img3_d);
		eb2.Add(img3);
		img3.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	gameOptions_click
	// 
	// Description: Game Options button is clicked, this function handles image swap and page
	//				change.
	//////////////////////////////////////////////////////////////////////////////////////////
	void gameOptions_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 3;
		eb3.Remove(img4_d);
		eb3.Add(img4);
		img4.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	mapEditor_click()
	// 
	// Description: 
	//////////////////////////////////////////////////////////////////////////////////////////
	void mapEditor_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 4;
		eb4.Remove(img5_d);
		eb4.Add(img5);
		img5.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	objectViewer_click()
	// 
	// Description: Object Viwewer button is clicked, this function handles image swap and page
	//				change.
	//////////////////////////////////////////////////////////////////////////////////////////
	void objectViewer_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 5;
		eb5.Remove(img6_d);
		eb5.Add(img6);
		img6.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	credits_click
	// 
	// Description: Credits button is clicked, this function handles image swap and page
	//				change.
	//////////////////////////////////////////////////////////////////////////////////////////
	void credits_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 6;
		eb6.Remove(img7_d);
		eb6.Add(img7);
		img7.Show();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	exit_click()
	// 
	// Description: Exit button is clicked, this function handles image swap and exit of 
	//				application.
	//////////////////////////////////////////////////////////////////////////////////////////
	void exit_click (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 7;
    }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	back_click()
	// 
	// Description: Responsible for changing back to main page when back buttons are pressed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void back_click (object o, EventArgs args)
	{
		nb.CurrentPage = 0;
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but2_down
	// 
	// Description: Handles but2 press event involving swaping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but2_down (object o, ButtonPressEventArgs args)
	{
		eb1.Remove(img2);
		eb1.Add(img2_d);
		img2_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but3_down()	
	// 
	// Description: Handles but3 press event involving swaping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but3_down (object o, ButtonPressEventArgs args)
	{
		eb2.Remove(img3);
		eb2.Add(img3_d);
		img3_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but4_down()
	// 
	// Description: Handles but4 press event involving swapping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but4_down (object o, ButtonPressEventArgs args)
	{
		eb3.Remove(img4);
		eb3.Add(img4_d);
		img4_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but5_down()
	// 
	// Description: Handles but5 press event involving swapping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but5_down (object o, ButtonPressEventArgs args)
	{
		eb4.Remove(img5);
		eb4.Add(img5_d);
		img5_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but6_down()
	// 
	// Description: TO BE ADDED
	//////////////////////////////////////////////////////////////////////////////////////////
	void but6_down (object o, ButtonPressEventArgs args)
	{
		eb5.Remove(img6);
		eb5.Add(img6_d);
		img6_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but7_down()
	// 
	// Description: Handles but5 press event involving swapping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but7_down (object o, ButtonPressEventArgs args)
	{
		eb6.Remove(img7);
		eb6.Add(img7_d);
		img7_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	but8_down()
	// 
	// Description: Handles but5 press event involving swapping of images and changing of page
	//				being displayed.
	//////////////////////////////////////////////////////////////////////////////////////////
	void but8_down (object o, ButtonPressEventArgs args)
	{
		eb7.Remove(img8);
		eb7.Add(img8_d);
		img8_d.Show();
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	back_click2()
	// 
	// Description: Handles changing of page to main.
	//////////////////////////////////////////////////////////////////////////////////////////
	void back_click2 (object o, ButtonReleaseEventArgs args)
	{
		nb.CurrentPage = 0;
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	allies_click()
	// 
	// Description: Responsible for image swap and flag manipulation of axis/alli button
	//				pressing.
	//////////////////////////////////////////////////////////////////////////////////////////
	void allies_click ( object o, ButtonReleaseEventArgs args)
	{
		// swap images and set flags!
		if(axis_flag == true)
		{
			eb_axis.Remove(axis_lit);
			eb_axis.Add(axis);
			axis.Show();
			eb_allies.Remove(allies);
			eb_allies.Add(allies_lit);
			allies_lit.Show();
			axis_flag = false;
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	axis_click()
	// 
	// Description: Responsible for image swap and flag manipulation of axis/alli button
	//				pressing.
	//////////////////////////////////////////////////////////////////////////////////////////
	void axis_click (object o, ButtonReleaseEventArgs args)
	{
		// swap images and set flags!
		if(axis_flag == false)
		{
			eb_allies.Remove(allies_lit);
			eb_allies.Add(allies);
			allies.Show();
			eb_axis.Remove(axis);
			eb_axis.Add(axis_lit);
			axis_lit.Show();
			axis_flag = true;
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	country_changed()
	// 
	// Description: Object viewer contains a drop down menu widget for changing countries.
	//////////////////////////////////////////////////////////////////////////////////////////
	void country_changed (object obj, EventArgs args)
	{
		// switch tree model when the country is changed, 
		// to display the new objects of that country
		for(int i =0;i < temp2.Length ;i++)
		{
			if(temp2[i] == countries_combo.Entry.Text)
			{
				tree_view.Model = tstore[i];
	            return;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	object_change()
	// 
	// Description: Responsible for retreiving information from the treeview containing
	//				what object was clicked.  Sets appropriate object image in right pane and
	//				appropriate object text description is also set to be displayed in the
	//				right pane below the image.
	//////////////////////////////////////////////////////////////////////////////////////////
	void object_change (object obj, EventArgs args)
	{
		
		TreeIter      iter;
		TreeModel     model;
		
		if (((TreeSelection)obj).GetSelected (out model, out iter))
		{
			// determine which row we are on
			string val = (string)model.GetValue (iter, 0);
			
			for(int i = 0 ; i < objects.Length;i++)
			{
				if(objects[i].name == val)
				{
					// add new image that was pressed
					imagebox.Remove(objImage);
					objImage = new Image(new Gdk.Pixbuf("./Menu/Objects/" + objects[i].image));
					imagebox.Add(objImage);
					objImage.Show();

					// adjust text display :)
					buf.Text = objects[i].desc;

							
					buf.ApplyTag(tt, buf.GetIterAtOffset(0), buf.GetIterAtOffset(buf.CharCount));

					
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	save_settings()
	// 
	// Description: Function is responsible for handling the output of the settings.ini file
	//				when the user presses the save button.  All game option variables are
	//				scanned in settings.ini file looking for the appropriate one, when found
	//				the variable is changed according to what the user modified in the game
	//				options page.
	//////////////////////////////////////////////////////////////////////////////////////////
	void save_settings (object o, ButtonReleaseEventArgs args)
	{
		try
		{
			FileStream file_read = new FileStream("settings.ini", FileMode.Open, FileAccess.Read);
			
			StreamReader sr = new StreamReader(file_read);
			
			string [] s = new string [1000];
			
		
			int i = 0;
			// while we are not at the end of the file
			while((s[i] = sr.ReadLine()) != null)
			{			
				for(int m = 0; m < 14; m++)
				{
					// determine if this is a line we want to check
					if(s[i].StartsWith(scan[m]))
					{
						s[i] = scan[m];
					
						for(int k = 0; k < 20 - scan[m].Length ; k++)
							s[i] += " ";
						
						// the following code will set the variables properly
						s[i] += "= ";
						if(scan[m] == "SCENERY_ELEMENTS")
							s[i] += combo_cov.Entry.Text;
						else if(scan[m] == "TREE_COVERAGE")
							s[i] += combo7.Entry.Text;
						else if(scan[m]== "WEATHER")
						{
							if(cb2.Active == true)                 
							{
								s[i] += "ENABLED";					//WEATHER
							}
							else
							{
								s[i] += "DISABLED";
							}
						}							
						else if(scan[m] == "SCROLL_SPEED")
							s[i] +=  hs2.Value;
						else if(scan[m] == "ROTATE_SPEED")
							s[i] +=  hs3.Value;
						else if(scan[m] == "SCREEN_SIZE")
							s[i] += SCREEN_SIZE;
						else if(scan[m] == "FULLSCREEN")
						{
							if(cb.Active == true)                 
							{
								s[i] += "TRUE";					
							}
							else
							{
								s[i] += "FALSE";
							}
						}							
						else if(scan[m] == "FILTERING")
							s[i] += combo3.Entry.Text.ToUpper();
						else if(scan[m] == "LOD_LEVEL")
							s[i] += combo_lod.Entry.Text.ToUpper();
						else if(scan[m] == "VOLUME")
							s[i] += hs.Value;
						else if(scan[m] == "SOUND")
						{
							if(cb_sound.Active == true)
								s[i] += "ENABLED";				//SOUND
							else
								s[i] += "DISABLED";
						}						
						else if(scan[m] == "MAX_SOUNDS")
							s[i] += combo5.Entry.Text;
						else if(scan[m] == "PENETRATION")
						{
							if(combo6.Entry.Text == "US Test Data")
								s[i] += "US";					//PENETRATION
							else
								s[i] += "RU";
						}
						
						else if(scan[m] == "DIAMETER_FIX")
						{
							if(cb_diameter.Active == true)
								s[i] += "ENABLED";
							else
								s[i] += "DISABLED";
						}
						
						else {}
						
						

					}
				}
				
				
				i++;
			}
			
			// close file 
			sr.Close();
			file_read.Close();

			// write new settings.ini
			FileStream file_write = new FileStream("settings.ini", FileMode.Create, FileAccess.Write);
			StreamWriter sw = new StreamWriter(file_write);	
			int w = 0;
			while(s[w] != null)
			{
				sw.WriteLine(s[w]);
				w++;
			}


			sw.Close();
			file_write.Close();
			Label label = new Label("Write to settings.ini successful!");			
			Dialog dialog = new Dialog("Success", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			label.Show();
				
			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}
		catch(Exception exception)
		{
			Label label = new Label("Write to settings.ini failed\n" + exception.ToString());			
			Dialog dialog = new Dialog("Failure", win, Gtk.DialogFlags.DestroyWithParent);
			dialog.Modal = true;
			dialog.VBox.PackStart(label, false, false, 0);
			label.Show();
				
			dialog.AddButton ("Close", ResponseType.Close);
			dialog.Run ();
			dialog.Destroy ();  
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Function:	play_snd()
	// 
	// Description: Function is for playing sound in windows.  This will later be implemented
	//				perhaps for button clicks.
	//////////////////////////////////////////////////////////////////////////////////////////
	void play_snd()
	{
		if(File.Exists("korps.ico"))
		{
			try
			{
				// play the sound from the selected filename
				if (!PlaySound( "C:\\heh.wav", IntPtr.Zero, SoundFlags.SND_FILENAME | SoundFlags.SND_ASYNC ))
				{
					;
				}
						
			}
			catch
			{			
				;
			}
		}
		else
		{
		}
	}


}

	