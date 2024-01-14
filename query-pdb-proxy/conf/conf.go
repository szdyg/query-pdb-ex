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
		//panic("QUERY_PDB_PROXY_MONGODB is empty")
		MongoDSN = "mongodb://mongodb:BPTFFcXU9l7r2qnt@10.57.1.22:27017/"
	}
	Port = os.Getenv("QUERY_PDB_PROXY_PORT")
	if len(Port) == 0 {
		Port = "6000"
	}
	PdbPath = os.Getenv("QUERY_PDB_PROXY_PATH")
	if len(PdbPath) == 0 {
		PdbPath = "/pdb"
	}
	QueryPdbServer = os.Getenv("QUERY_PDB_PROXY_REAL_SERVER")
}
