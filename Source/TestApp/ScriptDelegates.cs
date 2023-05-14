using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using API_STAT = System.UInt32;
using API_DEVICE_HANDLE = System.IntPtr;
using U32 = System.UInt32;
using U16 = System.UInt16;
using U8 = System.Byte;

namespace TestApp
{
//Scipt functions are stored in an array of Script commands as they
//are read in from a script file.
//The menu itself is just a list of MenuItems.
//Each MenuItem has several elements including the name
//of a Menu (if IsSubMenu is true) or the name of a
//menu function (IsSubMenu is false)
//MFDict is public in Form1.
//MenuDict is also public in Form1.
//So there is no structure to the storage of menus. It all hinges
//on the names of the items. When menu is created, the first menu
//encountered is put on the menu stack. It is the active menu.
//As submenus are called, their names are put on the menu stack
//which is just a stack of strings. When we exit a menu, we check
//if we are at top of menu (LIFO)stack. If not, we pop the top of
//the stack removing the last called menu. The previously entered
//menu name becomes current.
    public partial class Form1 : Form
    {
		private DFMenu TheMenu;
		public delegate U32 ScriptDelegate(U32 P1, U32 P2, U32 P3, U32 P4);//Script Delegate
		public class ScriptItem
	    {
			public MenuItem()
			{
				ValPrompt = new string[4];
			}
	    	public string Description;
	    	public string Name;
    		public string[] ValPrompt;
		}

        public class DFMenuPage
		{
			public DFMenuPage(string MenuTitle)
			{
				Title = MenuTitle;
				Items = new List<MenuItem>();
			}
			public string Name;
			public string Title;
			public List<MenuItem> Items;
		}
		public class DFMenu
        {
            public DFMenu()
            {
                MenuDict = new Dictionary<string, DFMenuPage>();
                DelDict = new Dictionary<string, MenuDelegate>();
                MenuStack = new Stack<DFMenuPage>();
                Params = new U32[4];
            	MenuReadErrors = 0;
		    	CurrentPage = null;
            }
            public int MenuReadErrors;
            public Dictionary<string, DFMenuPage> MenuDict;
			public Dictionary<string, MenuDelegate> DelDict;
		    public DFMenuPage CurrentPage;
		    public Stack<DFMenuPage> MenuStack;
 	    	public U32[] Params;
		}

		public void InitMenu()
		{
			TheMenu = new DFMenu();
			InitMenuDelegates();
		}

		public void AddScriptDelegate(string DelegateName, ScriptDelegate Delegate)
		{
			TheScript.DelDict.Add(DelegateName, Delegate);
		}

//Functions for building menu and testing --------------------------------------------------
		//Interpret the flags field of MITEM and FITEM 'macros'
		public U16 ParseFlags(string Flags)
		{
			U16 FlagsVal = 0;
			//some of these flags may be defunct
			if(Flags.Contains("HIDEMENU"))
    		{
    			FlagsVal |= 1;
    		}
			if(Flags.Contains("RTNVAL"))
    		{
    			FlagsVal |= 2;
    		}
			if(Flags.Contains("RTNVAL2"))
    		{
    			FlagsVal |= 4;
    		}
			if(Flags.Contains("MEN_STARTUP"))
    		{
    			FlagsVal |= 64;
    		}
			if(FlagsVal == 0)
			{ //maybe it is a numeric value
				if(!U16.TryParse(Flags, out FlagsVal))
				{
					return 0;
				}
			}
			return FlagsVal;
		}

		public bool MenuReadError(string ErrString, string Line, int LineNumber)
		{
            string MsgString = String.Format("{0} at line {1}:\n{2}", ErrString, LineNumber, Line);
            var Result = MessageBox.Show(MsgString, "Menu Read Error", MessageBoxButtons.OKCancel, MessageBoxIcon.Error);
            ++TheMenu.MenuReadErrors;
			if(Result == DialogResult.Cancel)
			{
			    return false;
            }
			return true;
		}


        private void MenuListBox_KeyDown(object sender, KeyEventArgs e)
        {
            int Index = (int)MenuListBox.SelectedIndex;

            switch (e.KeyCode)
            {
                case Keys.Escape:
                    PopMenu();
                    break;

                case Keys.Up:
                    if (Index > 0)
                    {
                        //  	--MenuListBox.SelectedIndex;
                    }
                    break;

                case Keys.Down:
                    if (Index < (MenuListBox.Items.Count - 1))
                    {
                        //    ++MenuListBox.SelectedIndex;
                    }
                    break;

                default:
                    return;
            }
	        DisplayMemPage(m_MemAddress);
        }

		public bool DrawMenu(string MenuName)
		{
			string ItemStr;
			DFMenuPage MenuPage = null;

			if(TheMenu.MenuDict.TryGetValue(MenuName, out MenuPage))
			{
				TheMenu.CurrentPage = MenuPage;
				MenuListBox.Items.Clear();
            	LMenuTitle.Text = MenuPage.Title;
                for(i = 0; i < MenuPage.Items.Count; ++i)
            	{
                    if(MenuPage.Items[i].IsSubmenu)
            		{
            			ItemStr = String.Format("<{0:C}> {1}", MenuPage.Items[i].Activator, MenuPage.Items[i].Description);
                        MenuListBox.Items.Add(ItemStr);
               		}
           		    else
            		{
            			ItemStr = String.Format("<{0}> {1}", MenuPage.Items[i].Activator, MenuPage.Items[i].Description);
                        MenuListBox.Items.Add(ItemStr);
               		}
            	}
                MenuListBox.SetSelected(0, true);
            }
			else
			{
	            string MsgStr;
        		MsgStr = String.Format("Cannot find {0} in menu pages", MenuName);
				MessageBox.Show(MsgStr);
	            return false;
			}
            return true;
        }

		public bool CheckMenu()
		{
            string MsgStr;
            bool Result = true;

			foreach(var Entry in TheMenu.MenuDict)
            {
            	for(i = 0; i < Entry.Value.Items.Count; ++i)
            	{
            		if(Entry.Value.Items[i].IsSubmenu)
            		{
            			if(!TheMenu.MenuDict.ContainsKey(Entry.Value.Items[i].Name))
                        {
							Result = false;
            				MsgStr = String.Format("Unknown menu page \"{0}\" called at item {1} in menu page {2}",
            										 Entry.Value.Items[i].Name, i + 1, Entry.Key);
							var MsgResult = MessageBox.Show(MsgStr, "Menu Error",
										 MessageBoxButtons.OKCancel, MessageBoxIcon.Error);
							if(MsgResult == DialogResult.Cancel)
							{
								return false;
							}
                        }
            		}
            		else
            		{
            			if(!TheMenu.DelDict.ContainsKey(Entry.Value.Items[i].Name))
                        {
							Result = false;
            				MsgStr = String.Format("Unknown function \"{0}\" called at item {1} in menu page {2}",
            										 Entry.Value.Items[i].Name, i + 1, Entry.Key);
							var MsgResult = MessageBox.Show(MsgStr, "Menu Error",
										 MessageBoxButtons.OKCancel, MessageBoxIcon.Error);
							if(MsgResult == DialogResult.Cancel)
							{
								return false;
							}

                        }
            		}
            	}
			}
            return Result;
        }

		public void PopMenu()
        {
            if(TheMenu.MenuStack.Count > 0)
            {
                TheMenu.CurrentPage = TheMenu.MenuStack.Pop();
                DrawMenu(TheMenu.CurrentPage.Name);
            }
        }

        public bool GetMenuParameter(int ItemIndex, int ParIdx)
        {
			if(ParIdx >= TheMenu.Params.Length)
			{
				MessageBox.Show("Parameter index error in GetMenuParameter");
				return false;
			}
            string Temp = TheMenu.CurrentPage.Items[ItemIndex].ValPrompt[ParIdx];
			if(StrToU32(Temp, out TheMenu.Params[ParIdx]))
			{//try to convert it as a number
				return true;
			}
			if(!GetInputString(Temp, out Temp))
			{ //else use it as a prompt
				return false;
			}
			if(StrToU32(Temp, out TheMenu.Params[ParIdx]))
			{  //try to convert what they input
				return true;
			}
            return false;
        }

        public void ExecuteMenuSelection()
        {
        //    new delegate U32 FunDelegate(U32 P1, U32 P2, U32 P3, U32 P4);//Menu Delegate
            string Message;
            int Index = MenuListBox.SelectedIndex;
            if (TheMenu.CurrentPage.Items[Index].IsSubmenu)
            {
                TheMenu.MenuStack.Push(TheMenu.CurrentPage);
                DrawMenu(TheMenu.CurrentPage.Items[Index].Name);
            }
            else
            {
                for (int i = 0; i < 4; ++i)
                {
	                TheMenu.Params[i] = 0;
                    if (TheMenu.CurrentPage.Items[Index].ValPrompt[i].Length > 0)
                    {
                        if(!GetMenuParameter(Index, i))
						{
							MessageBox.Show("Failed to get input parameter");
							return;
						}
                    }
                }
                MenuDelegate FunDelegate;
                if (TheMenu.DelDict.TryGetValue(TheMenu.CurrentPage.Items[Index].Name, out FunDelegate))
                {
                    U32 ReturnVal;
                    ReturnVal = FunDelegate(TheMenu.Params[0], TheMenu.Params[1], TheMenu.Params[2], TheMenu.Params[3]);
                	if((TheMenu.CurrentPage.Items[Index].Flags & RTNVAL) != 0)
                	{
                		Message = String.Format("Return value: {0}  (0X{0:X})", ReturnVal);
	                    MessageBox.Show(Message);
					}
				}
                else
                {
                    MessageBox.Show("Failed to find menu function delegate in dictionary");
                }
            }
        }

		public bool LoadMenu(string Path)
		{
			bool MenuOpen = false;
            char[] DelimChars = {'\'',',', '(', ')', '\t', '"'};
			string Line, MenuPageName = "";
            int LineNumber = 0;
            int MenuCount = 0;
            int ItemCount = 0;
			DFMenuPage MenuPage = null;
			MenuItem Item;
			// Read the file and display it line by line.
            System.IO.StreamReader file;
            try
            {
                file = new System.IO.StreamReader(@Path);
            }
            catch
            {
                MessageBox.Show("Cannot find menu definition file");
                return false;
            }
            while ((Line = file.ReadLine()) != null)
			{
			    ++LineNumber;
			    string[] Words = Line.Split(DelimChars, System.StringSplitOptions.RemoveEmptyEntries );
				if(Line.Length != 0)
				{
					for(int i = 0; i < Words.Length; ++i)
					{
						Words[i] = Words[i].Trim();
					}
					if(Words[0].Equals("MBEGIN", StringComparison.Ordinal))
					{
						if(Words.Length != 3)
			            {
			              	if(!MenuReadError("Bad MBEGIN", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
			            }
						MenuPageName = Words[1];
                        if(TheMenu.MenuDict.ContainsKey(MenuPageName))
                        {
			              	if(!MenuReadError("Duplicate menu page name encountered", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
                        }
                        MenuOpen = true;
						MenuPage = new DFMenuPage(Words[1]);
						MenuPage.Name = MenuPageName;
						ItemCount = 0;
					}
					else if(Words[0].Equals("MEND", StringComparison.Ordinal))
					{
						if(Words.Length != 1)
			            {
			              	if(!MenuReadError("Bad MEND", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
			            }
						if(!MenuOpen)
						{
							if(!MenuReadError("Menu End outside of menu", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
						}
						//Add the menu to the menu dictionary
						TheMenu.MenuDict.Add(MenuPageName, MenuPage);
                        ++MenuCount;
                        MenuOpen = false;
					}
					else if(Words[0].Equals("FITEM", StringComparison.Ordinal))
					{
						if(!MenuOpen)
						{
							if(!MenuReadError("FITEM outside of menu", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
						}
						if(Words.Length < 5)
			            {
			              	if(!MenuReadError("Bad FITEM", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
			            }
						Item = new MenuItem();
						Item.Activator = Words[1][0];
						Item.IsSubmenu = false;
	    				Item.Description = Words[2];
	    				Item.Name = Words[3];
    					Item.Flags = ParseFlags(Words[4]);
						for(int i = 0; i < 4; ++i)
						{
							if(Words.Length > (5 + i))
							{
								Item.ValPrompt[i] = Words[5 + i];
							}
							else
							{
								Item.ValPrompt[i] = "";
							}
						}
						MenuPage.Items.Add(Item);
						++ItemCount;
					}
					else if(Words[0].Equals("MITEM", StringComparison.Ordinal))
					{
						if(!MenuOpen)
						{
							if(!MenuReadError("MITEM outside of menu", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
						}
						if(Words.Length != 5)
			            {
			              	if(!MenuReadError("Bad MITEM", Line, LineNumber))
                            {
                                return false;
                            }
			            	continue;//go on at next line
			            }
						Item = new MenuItem();
						Item.Activator = Words[1][0];
						Item.IsSubmenu = true;
	    				Item.Description = Words[2];
	    				Item.Name = Words[3];
    					Item.Flags = ParseFlags(Words[4]);
						MenuPage.Items.Add(Item);
						++ItemCount;
					}
					if(ItemCount > 1000)
					{
                        MenuReadError("Fatal menu error, exceeded 1000 items", Line, LineNumber);
                        return false;
					}
					if(MenuCount > 1000)
					{
                        MenuReadError("Fatal menu error, exceeded 1000 menu pages", Line, LineNumber);
                        return false;
					}
				}
			}
			System.Console.WriteLine("\n ");
			System.Console.WriteLine("Processing Complete with {0} menu pages.\n", MenuCount);
			file.Close();
			// Suspend the screen.
            return true;
        }

//Make edits to the manual code above in the source file MDPart1.CS!!
//This is the end of MDPart1.CS, the manually generated part of final MenuDelegates.cs
//the following part is generated by ldfutil.exe----------------------------------------

		public U32 DGetSlaveParameter(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)GetSlaveParameter((U8)Par1, (U8)Par2);
		}

		public U32 DSetSlaveParameter(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)SetSlaveParameter((U8)Par1, (U16)Par2, (U16)Par3);
		}

		public U32 DSetServoUsec(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)SetServoUsec((U8)Par1, (U16)Par2);
		}

		public U32 DGetServoUsec(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)GetServoUsec((U8)Par1);
		}

		public U32 DGetProcGUID(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			GetProcGUID();
			return 0;
		}

		public U32 DCalcMemCRC(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)CalcMemCRC((U32)Par1, (U32)Par2);
		}

		public U32 DUploadToFile(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)UploadToFile((U32)Par1, (U32)Par2);
		}

		public U32 DFileDownload(U32 Par1, U32 Par2, U32 Par3, U32 Par4)
		{
			return (U32)FileDownload((U32)Par1);
		}

		public void InitMenuDelegates()
		{
			AddMenuDelegate("GetSlaveParameter", new MenuDelegate(DGetSlaveParameter));
			AddMenuDelegate("SetSlaveParameter", new MenuDelegate(DSetSlaveParameter));
			AddMenuDelegate("SetServoUsec", new MenuDelegate(DSetServoUsec));
			AddMenuDelegate("GetServoUsec", new MenuDelegate(DGetServoUsec));
			AddMenuDelegate("GetProcGUID", new MenuDelegate(DGetProcGUID));
			AddMenuDelegate("CalcMemCRC", new MenuDelegate(DCalcMemCRC));
			AddMenuDelegate("UploadToFile", new MenuDelegate(DUploadToFile));
			AddMenuDelegate("FileDownload", new MenuDelegate(DFileDownload));
		}
	}
}
