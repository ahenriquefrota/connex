//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MainFrm.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
 AnsiString TMainForm::version = "0.05";

//******************************************************************************
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner), dbConnection(nullptr)
{
	WindowState = wsMaximized;
	Application->OnHint = HintParaElSegundoPanel;
	LabelProgreso->Parent = StatusBar;
    LabelProgreso->Top = 4;
    LabelProgreso->Left = StatusBar->Panels->Items[0]->Width + 8;
	LabelProgreso->Caption = "";
}
/**
 *  Esta es para que el hint se dinuje en el segundo panel de la
 * StatusBar. Por defecto se dibuja en el primero y queda debajo
 * de la progressBar.
 */
void __fastcall TMainForm::HintParaElSegundoPanel(TObject *Sender)
{
	LabelProgreso->Caption = Application->Hint;
}

__fastcall TMainForm::~TMainForm()
{
    if (dbConnection) {
        delete dbConnection;
        dbConnection = nullptr;
    }
}

void TMainForm::InitializeDatabase()
{
    try {
        dbConnection = gConnections.Add_Connection(
            "postgres",
			lasOpciones.usuarioBD,
            lasOpciones.passwordBD,
            "localhost",
            "5432"
		);
		if (dbConnection && dbConnection->isConnected()) {
            StatusBar->Panels->Items[0]->Text = "Conexão com banco de dados estabelecida";
            OutputDebugString(L"InitializeDatabase: Conexão estabelecida com sucesso");
            dbConnection->setErrorHandler([this](const std::string& msg) {
                StatusBar->Panels->Items[1]->Text = msg.c_str();
                OutputDebugString(UnicodeString(msg.c_str()).c_str());
            });
        } else {
            StatusBar->Panels->Items[0]->Text = "Falha na conexão com o banco";
            OutputDebugString(L"InitializeDatabase: Falha na conexão com o banco");
        }
    } catch (const std::exception& e) {
        StatusBar->Panels->Items[0]->Text = "Erro ao conectar: " + UnicodeString(e.what());
        OutputDebugString((L"InitializeDatabase: Erro ao conectar: " + UnicodeString(e.what())).c_str());
	}

}








bool TMainForm::SetupConnection()
{
    dbConnection = gConnections.Add_Connection("postgres", "postgres", "Calaboca75", "localhost", "5432");
    return dbConnection != nullptr && dbConnection->isConnected();
}




void __fastcall TMainForm::FormShow(TObject *Sender)
{


	bool logado=ShowLogin();

	LoginForm = new TLoginForm(this, dbConnection, false);
	LoginForm->ShowModal();

	LabelProgreso->Caption = "Pronto";
	OutputDebugString(L"FormShow: Iniciando FormShow");
	if (Usuarios::usuarioActual().username.IsEmpty()) {
		OutputDebugString(L"FormShow: Usuário não logado, chamando ShowLogin");
		if (logado) {
			OutputDebugString(L"FormShow: ShowLogin retornou false");
			Close();
			return;
		}
	}
	OutputDebugString(L"FormShow: Atualizando StatusBar");
	StatusBar->Panels->Items[0]->Text = "Usuário logado: " + Usuarios::usuarioActual().username +
									   " (Acesso: " + UnicodeString(Usuarios::usuarioActual().acesso) + ")";
}

bool __fastcall TMainForm::ShowLogin()
{
	bool result=true;
	InitializeDatabase();
	ShowMessage("1");
	if (!dbConnection || !dbConnection->isConnected()) {
		OutputDebugString(L"ShowLogin: dbConnection inválida");
		ShowMessage(L"Conexão com o banco de dados não está ativa.");
		return false;
	}

	return result;


}


void TMainForm::actualizarPermisos() {
    Usuario actual = Usuarios::usuarioActual();
    Guardar1->Enabled = id_actual > 0 && !soloLectura;
}



void __fastcall TMainForm::Nuevo1Click(TObject *Sender)
{
	if (!checkCambiosOk())
		return;
	nuevoEscenario();
}
//---------------------------------------------------------------------------

void TMainForm::nuevoEscenario() {
	int nuevo_id = -1;
	//EscenarioDetalleFrame1->nuevoEscenario(nuevo_id, dbConnection);


}

bool TMainForm::checkCambiosOk() {

  return true;
}


void TMainForm::setearActual(int nuevo_id, AnsiString nuevo_nombre) {
    id_actual = nuevo_id;
    nombre_actual = nuevo_nombre;
	Caption = "Multiexport v" + version + " - " + nuevo_nombre + (soloLectura? " [modo sólo lectura]" : "");
    RecargarBtn->Enabled = true;
    GuardarBtn->Enabled = !soloLectura;
    GuardarCopiaBtn->Enabled = !soloLectura;
    Guardar1->Enabled = !soloLectura;
    Guardarcopia1->Enabled = !soloLectura;
}






void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (dbConnection) {
		dbConnection->destroy();
		delete dbConnection;
		dbConnection = nullptr;
	}
}
//---------------------------------------------------------------------------

