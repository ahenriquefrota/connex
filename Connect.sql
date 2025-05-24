DROP TABLE IF EXISTS usuarios;

CREATE TABLE usuarios (
    idpes SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password TEXT NOT NULL,
    nome VARCHAR(100),
    acesso INTEGER NOT NULL  -- grau de permissão: ex. 1=admin, 2=operador, etc.
);
INSERT INTO public.usuarios(
	username, password, nome, acesso)
	VALUES ('Frota','123','Antonio Henrique Souza Lopes Frota',1);


CREATE TABLE IF NOT EXISTS Escenarios (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(255) NOT NULL,
    descripcion TEXT,
    padre INTEGER,
    creacion TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    modificacion TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    fecha_resolucion TIMESTAMP,
    estado_resolucion VARCHAR(50),
    autor VARCHAR(100) NOT NULL,
    FOREIGN KEY (padre) REFERENCES Escenarios(id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS LockingEscenarios (
    id SERIAL PRIMARY KEY,
    username VARCHAR(100) NOT NULL,
    fecha_lock TIMESTAMP NOT NULL,
    FOREIGN KEY (username) REFERENCES usuarios(username) ON DELETE CASCADE
);

CREATE SEQUENCE fornecedor_idfor_seq
    START WITH 1
    INCREMENT BY 1
    NO MAXVALUE
    MINVALUE 0
    CACHE 1;


--
-- Name: fornecedor_idfor_seq; Type: SEQUENCE SET; Schema: alimentos; Owner: -
--

SELECT pg_catalog.setval('fornecedor_idfor_seq', 1, false);
CREATE TABLE fornecedor (
    idfor integer DEFAULT nextval('fornecedor_idfor_seq'::regclass) NOT NULL,
    idpes integer NOT NULL,
    idcli character varying(10) NOT NULL,
    razao_social character varying(50) NOT NULL,
    nome_fantasia character varying(50) NOT NULL,
    endereco character varying(40) NOT NULL,
    complemento character varying(30),
    bairro character varying(30) NOT NULL,
    cep character varying(8),
    cidade character varying(18) NOT NULL,
    uf character varying(2) NOT NULL,
    telefone character varying(11) NOT NULL,
    fax character varying(11),
    email character varying(40),
    contato character varying(40) NOT NULL,
    cpf_cnpj character varying(14) NOT NULL,
    inscr_estadual character varying(20),
    inscr_municipal character varying(20),
    tipo character(1) NOT NULL,
    CONSTRAINT ck_fornecedor CHECK (((tipo = 'F'::bpchar) OR (tipo = 'J'::bpchar)))
);

CREATE TABLE cadastro.medicamento (
    idmedicamento      SERIAL PRIMARY KEY,
    idpes              INTEGER NOT NULL REFERENCES cadastro.pessoa(idpes) ON DELETE RESTRICT,

    nome               TEXT NOT NULL,                             -- Nome comercial
    principio_ativo    TEXT NOT NULL,                             -- Princípio ativo
    concentracao       TEXT NOT NULL,                             -- Ex: "500 mg"
    forma_farmaceutica TEXT NOT NULL,                             -- Ex: "Comprimido", "Solução Oral"
    via_administracao  TEXT NOT NULL,                             -- Ex: "Oral", "Intravenosa"
    apresentacao       TEXT NOT NULL,                             -- Ex: "Caixa com 20 comprimidos"
    
    registro_anvisa    TEXT NOT NULL,                             -- Registro na ANVISA
    codigo_barras      TEXT,                                      -- EAN/GTIN
    classe_terapeutica TEXT,                                      -- Ex: "Antibiótico", "Analgésico"
    
    fabricante         TEXT,                                      -- Nome do fabricante (texto livre, pode duplicar info de idpes)
    origem             TEXT CHECK (origem IN ('Nacional', 'Importado')) DEFAULT 'Nacional',
    
    preco_maximo       NUMERIC(12,2),                             -- PMVG ou PMC
    data_validade      DATE,                                      -- Data de validade
    lote               TEXT,                                      -- Número do lote
    tipo_embalagem     TEXT,                                      -- Ex: "Primária", "Secundária"

    status             TEXT CHECK (status IN ('Ativo', 'Inativo')) DEFAULT 'Ativo',
    observacoes        TEXT,
    
    criado_em          TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    atualizado_em      TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE cadastro.telefone (
    idtelefone SERIAL PRIMARY KEY,
    idpes INTEGER NOT NULL REFERENCES cadastro.pessoa(idpes),
    tipo INTEGER NOT NULL, -- Ex: 1 = Residencial, 2 = Comercial, 3 = Celular
    ddd VARCHAR(3) NOT NULL,
    numero VARCHAR(10) NOT NULL,
    ramal VARCHAR(10),
    principal BOOLEAN DEFAULT FALSE,
    observacao TEXT,
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    atualizado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


CREATE TABLE compras.licitacao (
    idlicitacao SERIAL PRIMARY KEY,
    numero_processo VARCHAR(50) NOT NULL UNIQUE,
    modalidade VARCHAR(50) NOT NULL,           -- Ex: Pregão, Concorrência
    objeto TEXT NOT NULL,                      -- Descrição do objeto da licitação
    data_abertura DATE NOT NULL,
    hora_abertura TIME,
    situacao VARCHAR(30) DEFAULT 'Em andamento', -- Em andamento, Concluída, Cancelada, etc.
    idfornecedor INTEGER REFERENCES cadastro.pessoa(idpes), -- fornecedor vencedor
    idunidade INTEGER REFERENCES cadastro.unidade(id),      -- unidade que solicita
    valor_total NUMERIC(14,2),
    observacoes TEXT,
    criado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    atualizado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE estoque.produto (
    idproduto SERIAL PRIMARY KEY,
    nome VARCHAR(255) NOT NULL,
    tipo VARCHAR(50) NOT NULL,  -- 'medicamento', 'material', 'alimenticio', etc.
    descricao TEXT,
    ativo BOOLEAN DEFAULT TRUE,
    criado_em TIMESTAMP DEFAULT NOW()
);
CREATE TABLE licitacao.produto_licitacao (
    idproduto INTEGER REFERENCES estoque.produto(idproduto),
    idfornecedor INTEGER REFERENCES cadastro.pessoa(idpes),
    preco_unitario NUMERIC(10,2),
    marca VARCHAR(100),
    apresentacao VARCHAR(100),      -- Ex: Caixa com 20 comprimidos
    certificado_bpf BOOLEAN,        -- Boas Práticas de Fabricação
    validade_em_meses INTEGER,
    tempo_entrega_dias INTEGER,
    garantia_meses INTEGER,
    observacoes TEXT,
    PRIMARY KEY (idproduto, idfornecedor)
);
