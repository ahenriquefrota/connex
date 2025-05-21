//---------------------------------------------------------------------------

#ifndef MainFrmH
#define MainFrmH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.ComCtrls.hpp>

#include "dados/db_stuff.h"
#include "LoginFrm.h"
#include "Auxiliares.h"
#include "include/Opciones.h"








// Declaração antecipada para evitar dependências circulares
class TLoginForm;
class TUsuariosForm;


//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TPanel *Botonera;
	TBitBtn *CargarBtn;
	TBitBtn *RecargarBtn;
	TBitBtn *GuardarCopiaBtn;
	TBitBtn *GuardarBtn;
	TBitBtn *SalirBtn;
	TPanel *PanelEscenarioDetalle;
	TMainMenu *MainMenu1;
	TMenuItem *Escenario1;
	TMenuItem *Nuevo1;
	TMenuItem *N2;
	TMenuItem *Cargar1;
	TMenuItem *Guardar1;
	TMenuItem *Guardarcopia1;
	TMenuItem *Eliminar1;
	TMenuItem *N3;
	TMenuItem *Salir1;
	TMenuItem *Herramientas1;
	TMenuItem *Importarescenario1;
	TMenuItem *Exportarescenario1;
	TMenuItem *N1;
	TMenuItem *Resolver1;
	TMenuItem *N4;
	TMenuItem *Avanzarunperodo1;
	TMenuItem *Impresinmltiple1;
	TMenuItem *Sistema1;
	TMenuItem *Usuarios1;
	TMenuItem *Cambiodecontrasea1;
	TMenuItem *N8;
	TMenuItem *Opciones1;
	TMenuItem *Ayuda1;
	TMenuItem *Acercade1;
	TMenuItem *Tests1;
	TMenuItem *Test11;
	TLabel *LabelProgreso;
	TStatusBar *StatusBar;

	void __fastcall Nuevo1Click(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
	DB* dbConnection;
	Usuario authenticatedUser; // Armazena o usuário autenticado
	bool SetupConnection();
	bool checkCambiosOk();
	void nuevoEscenario();
	int id_actual;
    AnsiString nombre_actual;
	bool soloLectura;
	void setearActual(int nuevo_id, AnsiString nuevo_nombre);
	static AnsiString version;
	void actualizarPermisos();
    void __fastcall HintParaElSegundoPanel(TObject *Sender);
	void InitializeDatabase();
public:		// User declarations
    __fastcall TMainForm(TComponent* Owner);
	__fastcall ~TMainForm();
	bool __fastcall ShowLogin();

	Usuario getAuthenticatedUser() const { return authenticatedUser; } // Getter para o usuário
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
