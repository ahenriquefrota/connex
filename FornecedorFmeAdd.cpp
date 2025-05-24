#include <vcl.h>
#pragma hdrstop
#include "FornecedorFmeAdd.h"
#include "Pessoa.h"
#pragma package(smart_init)
#pragma resource "*.dfm"

TFornecedorFrameAdd *FornecedorFrameAdd;



// Função para validar string UTF-8
bool IsValidUTF8(const UnicodeString& str) {
    try {
        UTF8String utf8 = str;
        return true;
    } catch (Exception &e) {
        return false;
    }
}



__fastcall TFornecedorFrameAdd::TFornecedorFrameAdd(TComponent* Owner, DB* db, int id, const UnicodeString& tipo, TPessoaFrameAdd* mainFrame)
	: TFrame(Owner), dbConnection(db), idpes(id), tipo(tipo), MainFrame(mainFrame)
{
    ComboUf->Items->Add("AC");
    ComboUf->Items->Add("AL");
	ComboUf->Items->Add("AP");
    ComboUf->Items->Add("AM");
    ComboUf->Items->Add("BA");
    ComboUf->Items->Add("CE");
    ComboUf->Items->Add("DF");
    ComboUf->Items->Add("ES");
    ComboUf->Items->Add("GO");
    ComboUf->Items->Add("MA");
    ComboUf->Items->Add("MT");
    ComboUf->Items->Add("MS");
    ComboUf->Items->Add("MG");
    ComboUf->Items->Add("PA");
    ComboUf->Items->Add("PB");
    ComboUf->Items->Add("PR");
    ComboUf->Items->Add("PE");
    ComboUf->Items->Add("PI");
    ComboUf->Items->Add("RJ");
    ComboUf->Items->Add("RN");
    ComboUf->Items->Add("RS");
    ComboUf->Items->Add("RO");
    ComboUf->Items->Add("RR");
    ComboUf->Items->Add("SC");
    ComboUf->Items->Add("SP");
    ComboUf->Items->Add("SE");
    ComboUf->Items->Add("TO");
    ComboUf->ItemIndex = 0;
	PreencherCampos();
}

__fastcall TFornecedorFrameAdd::~TFornecedorFrameAdd()
{
}

void __fastcall TFornecedorFrameAdd::LimparCampos()
{
    EditRazaoSocial->Text = "";
    EditNomeFantasia->Text = "";
    EditCpfCnpj->Text = "";
    EditEndereco->Text = "";
    EditComplemento->Text = "";
    EditBairro->Text = "";
    EditCep->Text = "";
    EditCidade->Text = "";
    ComboUf->ItemIndex = 0;
    EditTelefone->Text = "";
    EditEmail->Text = "";
    EditContato->Text = "";
    EditInscrEstadual->Text = "";
    EditInscrMunicipal->Text = "";
    ErrorLabel->Caption = "";
}

void __fastcall TFornecedorFrameAdd::PreencherCampos()
{
    TPessoa pessoa(dbConnection);
    UnicodeString nome, tipoPessoa;
    if (pessoa.Consultar(idpes, nome, tipoPessoa)) {
        EditRazaoSocial->Text = nome;
        EditNomeFantasia->Text = nome;
        EditContato->Text = nome;
    }
    if (tipo == "F") {
        UnicodeString cpf, rg;
		TDateTime dataNasc;
        if (pessoa.ConsultarFisica(idpes, cpf, rg, dataNasc)) {
            EditCpfCnpj->Text = pessoa.FormatarCpf(cpf);
		}
    } else {
        UnicodeString cnpj, razaoSocial;
		if (pessoa.ConsultarJuridica(idpes, cnpj, razaoSocial)) {
			EditCpfCnpj->Text = pessoa.FormatarCnpj(cnpj);
			EditRazaoSocial->Text = razaoSocial;
			EditNomeFantasia->Text = razaoSocial;
		}
	}
}

void __fastcall TFornecedorFrameAdd::BtnSalvarClick(TObject *Sender)
{
    UnicodeString razaoSocial = EditRazaoSocial->Text.Trim();
    UnicodeString nomeFantasia = EditNomeFantasia->Text.Trim();
    UnicodeString cpfCnpj = EditCpfCnpj->Text.Trim();
    UnicodeString endereco = EditEndereco->Text.Trim();
    UnicodeString complemento = EditComplemento->Text.Trim();
    UnicodeString bairro = EditBairro->Text.Trim();
    UnicodeString cep = EditCep->Text.Trim();
    UnicodeString cidade = EditCidade->Text.Trim();
    UnicodeString uf = ComboUf->Text;
    UnicodeString telefone = EditTelefone->Text.Trim();
    UnicodeString email = EditEmail->Text.Trim();
    UnicodeString contato = EditContato->Text.Trim();
    UnicodeString inscrEstadual = EditInscrEstadual->Text.Trim();
    UnicodeString inscrMunicipal = EditInscrMunicipal->Text.Trim();

    TPessoa pessoa(dbConnection);
    UnicodeString cleanedTelefone = pessoa.LimparTelefone(telefone);
    UnicodeString cleanedCep = pessoa.LimparCep(cep);
    UnicodeString cleanedCpfCnpj = (tipo == "F") ? pessoa.LimparCpf(cpfCnpj) : pessoa.LimparCnpj(cpfCnpj);

    if (razaoSocial.IsEmpty() || cpfCnpj.IsEmpty() || endereco.IsEmpty() || bairro.IsEmpty() || cidade.IsEmpty() || uf.IsEmpty() || telefone.IsEmpty() || contato.IsEmpty()) {
        ErrorLabel->Caption = "Preencha todos os campos obrigatórios.";
        return;
    }

    if (razaoSocial.Length() > 100) {
        ErrorLabel->Caption = "Razão Social excede 100 caracteres.";
        return;
    }
    if (nomeFantasia.Length() > 100) {
        ErrorLabel->Caption = "Nome Fantasia excede 100 caracteres.";
        return;
    }
    if (endereco.Length() > 80) {
        ErrorLabel->Caption = "Endereço excede 80 caracteres.";
        return;
    }
    if (complemento.Length() > 30) {
        ErrorLabel->Caption = "Complemento excede 30 caracteres.";
        return;
    }
    if (bairro.Length() > 30) {
        ErrorLabel->Caption = "Bairro excede 30 caracteres.";
        return;
    }
    if (cleanedCep.Length() > 8) {
        ErrorLabel->Caption = "CEP excede 8 dígitos (sem formatação).";
        return;
    }
    if (cidade.Length() > 50) {
        ErrorLabel->Caption = "Cidade excede 50 caracteres.";
        return;
    }
    if (uf.Length() > 2) {
        ErrorLabel->Caption = "UF excede 2 caracteres.";
        return;
    }
    if (cleanedTelefone.Length() > 11) {
        ErrorLabel->Caption = "Telefone excede 11 dígitos (sem formatação).";
        return;
    }
    if (cleanedCpfCnpj.Length() > (tipo == "F" ? 11 : 14)) {
        ErrorLabel->Caption = (tipo == "F") ? "CPF excede 11 dígitos (sem formatação)." : "CNPJ excede 14 dígitos (sem formatação).";
        return;
    }
    if (email.Length() > 80) {
        ErrorLabel->Caption = "Email excede 80 caracteres.";
        return;
    }
    if (contato.Length() > 80) {
        ErrorLabel->Caption = "Contato excede 80 caracteres.";
        return;
    }
    if (inscrEstadual.Length() > 20) {
        ErrorLabel->Caption = "Inscrição Estadual excede 20 caracteres.";
        return;
    }
    if (inscrMunicipal.Length() > 20) {
        ErrorLabel->Caption = "Inscrição Municipal excede 20 caracteres.";
        return;
    }

    if (!IsValidUTF8(razaoSocial) || !IsValidUTF8(nomeFantasia) || !IsValidUTF8(endereco) ||
        !IsValidUTF8(complemento) || !IsValidUTF8(bairro) || !IsValidUTF8(cleanedCep) ||
        !IsValidUTF8(cidade) || !IsValidUTF8(uf) || !IsValidUTF8(cleanedTelefone) ||
        !IsValidUTF8(email) || !IsValidUTF8(contato) || !IsValidUTF8(cleanedCpfCnpj) ||
        !IsValidUTF8(inscrEstadual) || !IsValidUTF8(inscrMunicipal)) {
        ErrorLabel->Caption = "Um ou mais campos contêm caracteres inválidos para UTF-8.";
        return;
    }

    if (tipo == "F") {
        if (!pessoa.ModificarFisica(idpes, cleanedCpfCnpj, "", TDateTime::CurrentDate())) {
            ErrorLabel->Caption = "Erro ao atualizar pessoa física: " + dbConnection->getLastError();
            return;
        }
    } else {
        if (!pessoa.ModificarJuridica(idpes, cleanedCpfCnpj, razaoSocial)) {
            ErrorLabel->Caption = "Erro ao atualizar pessoa jurídica: " + dbConnection->getLastError();
            return;
        }
    }
    if (!pessoa.ModificarPessoa(idpes, razaoSocial, tipo)) {
        ErrorLabel->Caption = "Erro ao atualizar pessoa: " + dbConnection->getLastError();
        return;
    }

    if (pessoa.InserirFornecedor(idpes, razaoSocial, nomeFantasia, endereco, complemento, bairro, cleanedCep, cidade, uf, cleanedTelefone, email, contato, cleanedCpfCnpj, inscrEstadual, inscrMunicipal, tipo)) {
        ErrorLabel->Caption = "Fornecedor cadastrado com sucesso!";
        LimparCampos();

        // Exibir TFornecedorFrameList
        try {
            TPanel* parentPanel = MainFrame ? MainFrame->GetParentPanel() : dynamic_cast<TPanel*>(Parent);
            if (!parentPanel) {
                ShowMessage("Erro: Não foi possível determinar o painel pai.");
                return;
            }
            TFornecedorFrameList* listFrame = new TFornecedorFrameList(Parent, dbConnection, parentPanel);
            listFrame->Parent = parentPanel;
            listFrame->Top = this->Top;
            listFrame->Left = this->Left;
            listFrame->Width = this->Width;
            listFrame->Visible = true;
            this->Visible = false; // Esconder o frame atual
        } catch (Exception &e) {
            ShowMessage("Erro ao abrir lista de fornecedores: " + e.Message);
        }
    } else {
        ErrorLabel->Caption = "Erro ao cadastrar fornecedor: " + dbConnection->getLastError();
    }
}

void __fastcall TFornecedorFrameAdd::BtnCancelarClick(TObject *Sender)
{
    LimparCampos();
    Parent->Visible = false;
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
