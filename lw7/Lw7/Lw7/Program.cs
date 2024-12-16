using static System.Net.Mime.MediaTypeNames;

string ReadFileNameFromConsole()
{
    Console.WriteLine( "Enter file name:" );

    return Console.ReadLine()!;
}

IReadOnlyList<char> ReadCharactersToDeleteFromConsole()
{
    Console.WriteLine( "Enter characters you want to delete, ... to end" );

    string character = Console.ReadLine()!;
    List<char> charactersToDelete = [];

    while ( character != "..." )
    {
        charactersToDelete.Add( character[ 0 ] );

        character = Console.ReadLine()!;
    }

    return charactersToDelete;
}

string ReadTextFromFile( string fileName )
{
    string text;

    using ( StreamReader streamReader = new StreamReader( fileName ) )
    {
        text = streamReader.ReadToEnd();
    }

    return text;
}

string DeleteCharactersInText( string text, IReadOnlyList<char> charactersToDelete )
{
    string newText = "";

    for ( int i = 0; i < text.Length; i++ )
    {
        if ( !charactersToDelete.Contains( text[ i ] ) )
        {
            newText += text[ i ];
        }
    }

    return newText;
}

async Task WriteTextToFile( string text, string fileName )
{
    using ( StreamWriter streamWriter = new StreamWriter( fileName ) )
    {
        streamWriter.Write( text );
    }
}

string fileName = ReadFileNameFromConsole();
IReadOnlyList<char> charactersToDelete = ReadCharactersToDeleteFromConsole();
string editedText = DeleteCharactersInText( ReadTextFromFile( fileName ), charactersToDelete );
await WriteTextToFile( editedText, fileName );