package proxy

import (
	"bufio"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/gin-gonic/gin"
	"github.com/go-resty/resty/v2"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"go.uber.org/zap"
	"io"
	"net/http"
	"os"
	"path"
	"query-pdb-proxy/conf"
	"time"
)

var (
	mongoClient *mongo.Client
	logger      *zap.Logger
)

func InitRoute(r *gin.Engine) {
	logger, _ = zap.NewProduction()
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()
	var err error
	mongoClient, err = mongo.Connect(ctx, options.Client().ApplyURI(conf.MongoDSN).SetMaxPoolSize(200))
	if err != nil {
		logger.DPanic("connect db err", zap.Error(err))
		panic("connect db err")
	}

	r.POST("/symbol", proxySymbol)
	r.POST("/struct", proxyStruct)
	r.POST("/enum", proxyEnum)
}

type proxyParam struct {
	Name  string   `json:"name"`
	Msdl  string   `json:"msdl"`
	Query []string `json:"query"`
}

func proxySymbol(c *gin.Context) {
	proxyProc(c, "symbol")
}

func proxyStruct(c *gin.Context) {
	proxyProc(c, "struct")
}

func proxyEnum(c *gin.Context) {
	proxyProc(c, "enum")
}

func proxyProc(c *gin.Context, queryType string) {
	param := proxyParam{}
	c.BindJSON(&param)
	if len(param.Name) == 0 || len(param.Msdl) == 0 || len(param.Query) == 0 {
		c.JSON(http.StatusBadRequest, gin.H{
			"err": "invaild param",
		})
		return
	}
	result := make(map[string]interface{})
	for _, name := range param.Query {
		if len(name) == 0 {
			continue
		}
		// 1. find database
		off, err := queryPdbInfoFromDb(queryType, param.Name, param.Msdl, name)
		if err == nil {
			result[name] = off
			continue
		}
		// 2. download pdb
		pdbSavePath := path.Join(conf.PdbPath, param.Name, param.Msdl, param.Name)
		err = downloadPdb(param.Name, param.Msdl, pdbSavePath)
		if err != nil {
			logger.Error("download pdb error",
				zap.String("name", param.Name),
				zap.String("msdl", param.Msdl),
				zap.Error(err))
			defer os.Remove(pdbSavePath)
		}
		// 3. query real server
		off, err = querySymbolFromServer(queryType, param.Name, param.Msdl, name)
		if err != nil {
			logger.Error("query server error",
				zap.String("name", param.Name),
				zap.String("msdl", param.Msdl),
				zap.Error(err))
			continue
		}
		result[name] = off

		// 4. save result to db
		SaveQueryPdbServerToDb(queryType, param.Name, param.Msdl, result)
	}
	c.JSON(http.StatusOK, result)
}

func queryPdbInfoFromDb(queryType string, pdbName string, msdl string, queryName string) (info interface{}, err error) {
	pdbId := pdbName + "." + msdl
	collection := mongoClient.Database("pdb").Collection(queryType)
	filter := bson.M{"pdb_id": pdbId}
	var pdbSetInfo bson.M
	err = collection.FindOne(context.TODO(), filter).Decode(&pdbSetInfo)
	if err != nil {
		logger.Warn("not find pdb info in db",
			zap.String("name", pdbName),
			zap.String("msdl", queryType),
			zap.String("query", queryName),
			zap.Error(err))
		return
	}
	if vaule, ok := pdbSetInfo[queryName]; ok {
		info = vaule
		return
	} else {
		err = errors.New(fmt.Sprintf("[%s] [%s] [%s] is not found in db", pdbId, queryType, queryName))
	}
	return
}

func querySymbolFromServer(queryType string, pdbName string, msdl string, queryName string) (info interface{}, err error) {
	serverUrl := conf.QueryPdbServer + "/" + queryType
	pdbId := pdbName + "." + msdl
	param := proxyParam{
		Name:  pdbName,
		Msdl:  msdl,
		Query: []string{queryName},
	}
	paramJson, err := json.Marshal(param)

	client := resty.New()
	resp, err := client.R().EnableTrace().
		SetHeader("Content-Type", "application/json").
		SetBody(paramJson).
		Post(serverUrl)
	if err != nil {
		logger.Error("req real server err",
			zap.String("name", pdbName),
			zap.String("msdl", queryType),
			zap.String("query", queryName),
			zap.Error(err))
		return
	}

	if resp.StatusCode() != http.StatusOK {
		err = errors.New(fmt.Sprintf("query-pdb server error: %d", resp.StatusCode()))
		logger.Error("query-pdb server error", zap.Error(err))
		return
	}
	result := make(map[string]interface{})
	if err = json.Unmarshal(resp.Body(), &result); err != nil {
		return
	}

	if value, ok := result[queryName]; ok {
		info = value
	} else {
		err = errors.New(fmt.Sprintf("[%s] [%s] [%s] is not found in query-qdb-server", pdbId, queryType, queryName))
	}

	return
}

func downloadPdb(pdbName string, msdl string, pdbSavePath string) (err error) {
	if _, err = os.Stat(pdbSavePath); err == nil {
		// already download
		return
	}
	pdbUrl := conf.MsdlServer + pdbName + "/" + msdl + "/" + pdbName
	res, err := http.Get(pdbUrl)
	if err != nil {
		logger.Error("http get pdb err", zap.Error(err), zap.String("url", pdbUrl))
		return
	}
	defer res.Body.Close()

	if res.StatusCode != 200 {
		err = errors.New(fmt.Sprintf("msdl server err: %d", res.StatusCode))
		logger.Error("msdl server err", zap.Error(err))
		return
	}

	os.Remove(pdbSavePath)
	pdbDir := path.Dir(pdbSavePath)
	os.MkdirAll(pdbDir, 0644)

	pdbFile, err := os.Create(pdbSavePath)
	defer pdbFile.Close()
	if err != nil {
		logger.Error("create pdb file err", zap.Error(err), zap.String("path", pdbSavePath))
		return
	}

	buf := bufio.NewWriter(pdbFile)
	_, err = io.Copy(buf, res.Body)
	if err != nil {
		logger.Error("copy pdb file buf err", zap.Error(err), zap.String("path", pdbSavePath))
		return
	}

	err = buf.Flush()
	if err != nil {
		logger.Error("flush pdb file buf err", zap.Error(err), zap.String("path", pdbSavePath))
		return
	}
	return
}

func SaveQueryPdbServerToDb(queryType string, pdbName string, msdl string, pdbInfo map[string]interface{}) (err error) {
	pdbId := pdbName + "." + msdl
	collection := mongoClient.Database("pdb").Collection(queryType)

	pdbInfo["pdb_id"] = pdbId
	updateInfo := bson.M{
		"$set": pdbInfo,
	}
	filter := bson.M{"pdb_id": pdbId}

	upsert := true
	updateOptions := options.UpdateOptions{
		Upsert: &upsert,
	}
	_, err = collection.UpdateOne(context.TODO(), filter, updateInfo, &updateOptions)
	if err != nil {
		logger.Error("save db error", zap.Error(err))
	}

	return
}
