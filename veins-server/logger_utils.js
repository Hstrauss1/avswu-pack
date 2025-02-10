import pino from 'pino';
import pretty from 'pino-pretty';

// messages of types debug, info, & errors
//   { level: 'debug' },
// messages of types info, & errors
//   { level: 'info' },
// messages of type, errors only
//   { level: 'error' },

const logger = pino(
  { level: 'info' },
  pretty({
    sync: true,
    colorize: true,
    translateTime: 'mm-dd-yyyy:HH-MM:ss'
  })
);

export default logger;
