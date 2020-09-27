const express = require("express");
const bodyParser = require("body-parser");
const request = require("request");
const fs = require("fs");
const app = express();
const port = 1314;

app.use(
  bodyParser.raw({
    limit: "2mb",
    type: "*/*",
  })
);

app.post("*", (req, res) => {
  fs.writeFileSync("audio.raw", req.body);
  request.post(
    "https://api.wit.ai/speech?v=20200927",
    {
      headers: {
        authorization: req.headers.authorization,
        "content-type": req.headers["content-type"],
      },
      body: req.body,
    },
    (err, apiResponse, body) => {
      if (err) {
        return console.log(err);
      }
      const results = JSON.parse(body);
      if (results.intents && results.intents.length > 0) {
        const { confidence, name } = results.intents[0];
        if (results.traits && results.traits["wit$on_off"]) {
          const {
            value: traitValue,
            confidence: traitConfidence,
          } = results.traits["wit$on_off"][0];
          if (results.entities && results.entities["device:device"]) {
            const {
              confidence: deviceConfidence,
              value: deviceName,
            } = results.entities["device:device"][0];
            res.send(
              `Intent:${name}(${confidence})\nDevice:${deviceName}(${deviceConfidence})\nAction:${traitValue}(${traitConfidence})`
            );
            return;
          }
        }
      }
      res.send("not_understood");
    }
  );
});

app.listen(port, "0.0.0.0", () => {
  console.log(`Speech proxy app listening at http://0.0.0.0:${port}`);
});
