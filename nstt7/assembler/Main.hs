{-# LANGUAGE BinaryLiterals #-}

module Main where

import Data.Binary.Put
import qualified Data.ByteString.Lazy as BL
import Data.Either.Extra
import Encoding
import Options.Applicative
import Parsing
import System.FilePath
import System.IO
import Text.Megaparsec

source :: Parser FilePath
source =
  strArgument
    (metavar "SOURCE" <> help "Source file to assemble")

main :: IO ()
main = do
  filename <-
    execParser $
      info
        source
        (fullDesc <> progDesc "Assemble a BLRISC program" <> header "blasm - a BLRISC assembler")
  input <- readFile' filename
  either
    putStrLn
    (BL.writeFile (filename -<.> "bin") . runPut)
    (mapLeft errorBundlePretty (parse parseProgram filename input) >>= encodeProgram)
