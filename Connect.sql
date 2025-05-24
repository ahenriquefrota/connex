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


