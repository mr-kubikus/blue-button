//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <vector>
#include <windows.h>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"
#include "Unit3.h"
#include "Unit4.h"
#include "adds.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "XPManifest"
#pragma link "acSlider"
#pragma link "sButton"
#pragma link "sGroupBox"
#pragma link "sListBox"
#pragma link "sPanel"
#pragma link "sSkinManager"
#pragma link "sComboBox"
#pragma resource "*.dfm"

#define STRMAXLEN   80

//=============================================================================

TForm1 *Form1;
HINSTANCE ftMscLib;
DWORD errCode;
HANDLE  fthdl;
char LibVersion[STRMAXLEN];
char ComPortName[STRMAXLEN];
std::vector<ButtonListItem> buttonList;
bool Saved = true;
bool Edit = false;
bool Deleted = false;
String Version = "1.0.00 beta";


enum MotorIdx {MOTOR_1 = 0, MOTOR_2, MOTOR_3, MOTOR_4};
enum PwmOut {OUT1 = 0, OUT2, OUT3, OUT4, OUT5, OUT6, OUT7, OUT8};

//=============================================================================

typedef DWORD (*FTXGETLIBVERSION)();
       FTXGETLIBVERSION ftxGetLibVersion;

typedef DWORD (*FTXGETLIBVERSIONSTR)(LPSTR,DWORD);
       FTXGETLIBVERSIONSTR ftxGetLibVersionStr;

typedef DWORD (*FTXINITLIB)();
       FTXINITLIB ftxInitLib;

typedef HANDLE (*FTXOPENCOMDEVICE)(char *, DWORD, DWORD *);
       FTXOPENCOMDEVICE ftxOpenComDevice;

typedef DWORD (*FTXSTARTTRANSFERAREA)(HANDLE);
       FTXSTARTTRANSFERAREA ftxStartTransferArea;

typedef DWORD (*SETFTMOTORCONFIG)(HANDLE,int,int,bool);
       SETFTMOTORCONFIG SetFtMotorConfig;

typedef DWORD (*SETOUTPWMVALUES)(HANDLE,int,int,int);
       SETOUTPWMVALUES SetOutPwmValues;

typedef DWORD (*FXTSTOPTRANSFERAREA)(HANDLE);
       FXTSTOPTRANSFERAREA ftxStopTransferArea;

typedef DWORD (*FXTCLOSEDEVICE)(HANDLE);
       FXTCLOSEDEVICE ftxCloseDevice;

typedef DWORD (*FXTCLOSELIB)();
       FXTCLOSELIB ftxCloseLib;

typedef DWORD (*GETCOMSTATUS)(HANDLE);
       GETCOMSTATUS GetComStatus;

typedef DWORD (*GETAVAILABLECOMPORTS)(int);
       GETAVAILABLECOMPORTS GetAvailableComPorts;

typedef DWORD (*ENUMCOMPORTS)(DWORD,LPSTR,DWORD);
       ENUMCOMPORTS EnumComPorts;

CHAR KeyToName(int Key) {
        if ((Key>=65)&&(Key<=90)||(Key>=48)&&(Key<=57))
                return Char(Key);
        return 0;
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
/*
void stringGridUnselect() {
        TGridRect clear;
        Form1->StringGrid1->Selection = clear;
}
*/

void __fastcall TForm1::SetBI(bool BInterface)
{
        BI = !BInterface;
        sButton1->Enabled = BI;
        sButton2->Enabled = BI;
        sButton3->Enabled = BI;
    //    StringGrid1->Enabled = BI;
}

bool __fastcall InitftMscLib() {

       ftxGetLibVersion  =
       (FTXGETLIBVERSION)GetProcAddress(ftMscLib,"ftxGetLibVersion");

       ftxGetLibVersionStr  =
       (FTXGETLIBVERSIONSTR)GetProcAddress(ftMscLib,"ftxGetLibVersionStr");

       ftxInitLib  =
       (FTXINITLIB)GetProcAddress(ftMscLib,"ftxInitLib");

       ftxOpenComDevice  =
       (FTXOPENCOMDEVICE)GetProcAddress(ftMscLib,"ftxOpenComDevice");

       ftxStartTransferArea  =
       (FTXSTARTTRANSFERAREA)GetProcAddress(ftMscLib,"ftxStartTransferArea");

       SetFtMotorConfig  =
       (SETFTMOTORCONFIG)GetProcAddress(ftMscLib,"SetFtMotorConfig");

       SetOutPwmValues  =
       (SETOUTPWMVALUES)GetProcAddress(ftMscLib,"SetOutPwmValues");

       ftxStopTransferArea  =
       (FXTSTOPTRANSFERAREA)GetProcAddress(ftMscLib,"ftxStopTransferArea");

       ftxCloseDevice  =
       (FXTCLOSEDEVICE)GetProcAddress(ftMscLib,"ftxCloseDevice");

       ftxCloseLib  =
       (FXTCLOSELIB)GetProcAddress(ftMscLib,"ftxCloseLib");

       GetComStatus  =
       (GETCOMSTATUS)GetProcAddress(ftMscLib,"GetComStatus");

       GetAvailableComPorts  =
       (GETAVAILABLECOMPORTS)GetProcAddress(ftMscLib,"GetAvailableComPorts");

       EnumComPorts  =
       (ENUMCOMPORTS)GetProcAddress(ftMscLib,"EnumComPorts");

       return true;
}

void __fastcall TForm1::FormCreate(TObject *Sender)
{
        StringGrid1->Cells[0][0]="�������";
        StringGrid1->Cells[1][0]="�����";
        StringGrid1->Cells[2][0]="�������";
        //stringGridUnselect();

        ftMscLib = LoadLibrary("ftMscLib.dll");
        if (ftMscLib) {
                if (!InitftMscLib())
                        ShowMessage("������������ ������ ����������");
        } else {
                sButton4->Enabled = false;
                Form1->Show();
                ShowMessage("�� ������� ����� \"ftMscLib.dll\"");
        }


}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
        ShowMessage(ftxGetLibVersion());
}
//---------------------------------------------------------------------------


void __fastcall TForm1::sButton1Click(TObject *Sender)
{
        Form2->sRadioButton1->Checked = False;
        Form2->sRadioButton2->Checked = False;
        Form2->sComboBox1->ItemIndex = 0;
        Form2->sEdit1->Text = "������� �������...";
        Form2->ShowModal();
}
//---------------------------------------------------------------------------


static void InitAllOutput(int status) {

    //  set all Motor output OFF, PWM output ON or Motor output ON, PWM output OFF
    SetFtMotorConfig(fthdl, TA_LOCAL, MOTOR_1, (status == 1));
    SetFtMotorConfig(fthdl, TA_LOCAL, MOTOR_2, (status == 1));
    SetFtMotorConfig(fthdl, TA_LOCAL, MOTOR_3, (status == 1));
    SetFtMotorConfig(fthdl, TA_LOCAL, MOTOR_4, (status == 1));
}

void __fastcall TForm1::sButton5Click(TObject *Sender)
{
        //  get library version
        //    ftxGetLibVersionStr(LibVersion,STRMAXLEN);
        //  library initialization
        errCode = ftxInitLib();

        String shortName = sComboBox1->Text.SubString(0,sComboBox1->Text.Pos(" "));

        char * ComPortName = shortName.c_str();

        //  open COM port
        fthdl = ftxOpenComDevice(ComPortName, 38400, &errCode);

        if (errCode == FTLIB_ERR_SUCCESS) {
                sButton4->Enabled = False;
                sComboBox1->Enabled = False;
                sButton5->Enabled = False;
                sButton6->Enabled = True;
                sButton7->Enabled = True;
        } else {
        //  error case
                String str = "�� ������� ������������ � ����������� ����� \""+sComboBox1->Text+"\"";
                ShowMessage(str);
        }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::sButton7Click(TObject *Sender)
{
        //  starting Transfer Area
        errCode = ftxStartTransferArea(fthdl);

        if (errCode == FTLIB_ERR_SUCCESS) {
                sButton8->Enabled = True;
                sButton6->Enabled = False;
                sButton7->Enabled = False;
                BlockInterface = True;
                ShowMessage("Transfer Area ��������...");
        } else {
        //  error case
                ShowMessage("Error: �� ������� ��������� Transfer Area!");
        }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::sButton8Click(TObject *Sender)
{
        //  stop Transfer Area
        ftxStopTransferArea(fthdl);

        sButton8->Enabled = False;
        sButton6->Enabled = True;
        sButton7->Enabled = True;

        BlockInterface = False;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::sButton6Click(TObject *Sender)
{
        //  closing port
        errCode = ftxCloseDevice(fthdl);
        //  close library
        ftxCloseLib();

        sButton4->Enabled = True;
        sComboBox1->Enabled = True;
        sButton5->Enabled = True;
        sButton6->Enabled = False;
        sButton7->Enabled = False;
}
//---------------------------------------------------------------------------













void __fastcall TForm1::sButton3Click(TObject *Sender) {
        if (StringGrid1->Cells[0][1]!=""&&MessageDlg("�� �������?",mtConfirmation,TMsgDlgButtons()<<mbYes<<mbNo,0)!=6) return;
        int all = 0;
        for (int i=StringGrid1->Selection.Bottom;i>=StringGrid1->Selection.Top;i--) {
                if (StringGrid1->RowCount>StringGrid1->Selection.Bottom) {
                        if (buttonList.size()>0) buttonList.erase(buttonList.begin()+i);
                        StringGrid1->Rows[i] = StringGrid1->Rows[i+1];
                        for (int j=0;j<StringGrid1->ColCount;j++)
                                StringGrid1->Cells[j][i+1] = "";
                }
                all++;
        }
        if (StringGrid1->RowCount-all<2) all = StringGrid1->RowCount-2;
        StringGrid1->RowCount = StringGrid1->RowCount-all;

        Form1->StringGrid1->SetFocus();
        if (Form1->StringGrid1->Cells[0][1]!="") Saved = false;

}
//---------------------------------------------------------------------------


void __fastcall TForm1::StringGrid1KeyPress(TObject *Sender, char &Key)
{
        TGridRect newRect;
        if (Key==1) {
                newRect.Bottom = 1;
                newRect.Top = StringGrid1->RowCount-1;
                newRect.Left = 0;
                newRect.Right = StringGrid1->ColCount-1;
                StringGrid1->Selection = newRect;
        }
}
//---------------------------------------------------------------------------




void __fastcall TForm1::N7Click(TObject *Sender)
{
        Close();
}
//---------------------------------------------------------------------------

void __fastcall Control(int key,bool Speed) {
        int direct;
        int speed;
        for (int i=0;i<buttonList.size();i++) {
                if (key==buttonList[i].key) {
                        direct = buttonList[i].direct ? 1: 0;
                        speed = Speed ? 512 : 0;
                        SetOutPwmValues(fthdl, TA_LOCAL, buttonList[i].motor*2+direct, speed);
                }
        }
}

void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        Control(Key,true);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        Control(Key,false);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1Click(TObject *Sender)
{
        int answer;
        if (!Saved) {
                answer = MessageDlg("��������� ������� ������ ������?",mtConfirmation,TMsgDlgButtons()<<mbYes<<mbNo<<mbCancel,0);
                switch(answer) {
                        case 6: ShowMessage("���������");
                        break;

                        case 2: return;
                        break;
                }
        }
        buttonList.clear();
        for (int i=1;i<StringGrid1->RowCount;i++) {
                StringGrid1->Rows[i]->Clear();
        }
        StringGrid1->RowCount = 2;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N9Click(TObject *Sender)
{
        Form3->Show();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N11Click(TObject *Sender)
{
        AnsiString str = "http://andrew-pridex.com/blue-button/?v="+Version;
        ShellExecute(0,"open",str.c_str(),"","", 1);
//        Form4->Show();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::sButton4Click(TObject *Sender)
{
        sComboBox1->Clear();
        char *portName = new char[256];
        for (int i=0;i<GetAvailableComPorts(0);i++) {
                EnumComPorts(i,portName,150);
                sComboBox1->Items->Add(portName);
        }
        if (sComboBox1->Items->Count==0) {
                ShowMessage("����� �� �������");
                return;
        }
        sComboBox1->ItemIndex = 0;
        sComboBox1->Enabled = true;
        if (!sButton6->Enabled) sButton5->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::sButton2Click(TObject *Sender)
{
        int index = StringGrid1->Row-1;
        Form2->sRadioButton1->Checked = False;
        Form2->sRadioButton2->Checked = False;
        if (buttonList[index].direct)
                Form2->sRadioButton1->Checked = True;
        else
                Form2->sRadioButton2->Checked = True;
        Form2->sComboBox1->ItemIndex = buttonList[index].motor;
        Form2->sEdit1->Text = StringGrid1->Cells[0][StringGrid1->Row];
        Edit = True;
        Form2->ShowModal();

}
//---------------------------------------------------------------------------


void __fastcall TForm1::StringGrid1DrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
        if (StringGrid1->Cells[0][StringGrid1->Row]!="")
                sButton2->Enabled = True;
        else
                sButton2->Enabled = False;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::StringGrid1KeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        if (Key==46&&!Deleted) {
                sButton3Click(Sender);
                Deleted = True;
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::StringGrid1KeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
        if (Key==46) Deleted = false;
}
//---------------------------------------------------------------------------


