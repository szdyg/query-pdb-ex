package conf

import (
	"os"
)

var (
	MongoDSN       string
	Port           string
	PdbPath        string
	QueryPdbServer string
	MsdlServer     string
)

func init() {
	MongoDSN = os.Getenv("QUERY_PDB_PROXY_MONGODB")
	if len(MongoDSN) == 0 {
		panic("QUERY_PDB_PROXY_MONGODB is empty")
	}
	Port = os.Getenv("QUERY_PDB_PROXY_PORT")
	if len(Port) == 0 {
		Port = "80"
	}
	PdbPath = os.Getenv("QUERY_PDB_PROXY_PATH")
	if len(PdbPath) == 0 {
		PdbPath = "/pdb"
	}
	QueryPdbServer = os.Getenv("QUERY_PDB_PROXY_REAL_SERVER")
	if len(MongoDSN) == 0 {
		panic("QUERY_PDB_PROXY_REAL_SERVER is empty")
	}
	MsdlServer = os.Getenv("QUERY_PDB_PROXY_MSDL_SERVER")
	if len(MsdlServer) == 0 {
		MsdlServer = "http://msdl.microsoft.com/download/symbols/"
	}
}
