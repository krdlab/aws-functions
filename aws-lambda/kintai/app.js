// Copyright (c) 2017 Sho Kuroda
// Released under the MIT license.
const Spreadsheet = require('google-spreadsheet');
const moment      = require('moment-timezone');
const async       = require('async');

const DOC_ID = process.env.GOOGLE_SPREAD_SHEET_ID;
const CREDENTIALS_JSON = process.env.GOOGLE_CREDENTIALS_JSON;

const TZ = 'Asia/Tokyo';
const momentz = () => moment().tz(TZ);

const row = (start) => {
  const c = momentz().startOf('day');
  const s = moment.tz(`${c.year()}${start}`, TZ);
  const duration = moment.duration(c.valueOf() - s.valueOf());
  return 14 + duration.asDays();
};

const time = () => momentz().format('HH:mm');

exports.handler = (event, context, callback) => {
  const attendance = event.path === '/1';

  const doc = new Spreadsheet(DOC_ID);
  var sheet;

  async.waterfall([
    (next) => {
      const credentials = require(CREDENTIALS_JSON);
      doc.useServiceAccountAuth(credentials, next);
    },
    (next) => doc.getInfo(next),
    (info, next) => {
      sheet = info.worksheets[0];

      const startDate = sheet.title.trim().split('-')[0];
      const pos = row(startDate);
      const options = {
        'min-row': pos,
        'max-row': pos,
        'min-col': 1,
        'max-col': 15,
        'return-empty': true
      };
      sheet.getCells(options, next);
    },
    (cells, next) => {
      if (attendance) {
        cells[4].value = '出勤';
        cells[8].value = time();
        cells[14].value = '1:00';
      } else {
        cells[11].value = time();
      }
      sheet.bulkUpdateCells(cells, next);
    }
  ], (err) => {
    const code = err == null ? 200 : 502;
    callback(err, {statusCode: code, headers: {}, body: null});
  });
};
