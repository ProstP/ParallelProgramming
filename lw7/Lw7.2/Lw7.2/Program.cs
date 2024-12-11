using System.Diagnostics;
using System.Net;
using Newtonsoft.Json.Linq;

const string ApiUrl = "https://dog.ceo/api/breeds/image/random";
const int ImageCount = 10;

HttpClient client = new();
Stopwatch stopwatch = new Stopwatch();

Console.WriteLine( "Start synchronous download" );

stopwatch.Start();
await DownloadRandomDogImages( ApiUrl, client, ImageCount );
stopwatch.Stop();

Console.WriteLine( "Time: {0}ms", stopwatch.ElapsedMilliseconds );
Console.WriteLine();

stopwatch.Reset();

Console.WriteLine( "Start asynchronous download" );
stopwatch.Start();
await DownloadRandomDogImagesAsync( ApiUrl, client, ImageCount );
stopwatch.Stop();

Console.WriteLine( "Time: {0}ms", stopwatch.ElapsedMilliseconds );

async Task DownloadRandomDogImages( string apiUrl, HttpClient client, int count )
{
    for ( int i = 0; i < count; i++ )
    {
        int num = i;
        await DownloadRandomDogImageFromUrlAsync( apiUrl, client, num, "Sync" );
    }
}

async Task DownloadRandomDogImagesAsync( string apiUrl, HttpClient client, int count )
{
    Task[] tasks = new Task[ ImageCount ];

    for ( int i = 0; i < ImageCount; i++ )
    {
        int taskNums = i;
        tasks[ i ] = DownloadRandomDogImageFromUrlAsync( apiUrl, client, taskNums, "Async" );
    }

    await Task.WhenAll( tasks );
}

async Task DownloadRandomDogImageFromUrlAsync( string apiUrl, HttpClient client, int num, string type )
{
    Response response = await GetRandomDogImageUrlAsync( apiUrl, client );

    if ( response.status != "success" )
    {
        Console.WriteLine( "Error in get image url, {0}", num );

        return;
    }

    string fileName = type + num.ToString() + ".jpg";
    Console.WriteLine( "Task {0} start download file from url:{1}", num, response.imageUrl );
    DownloadImageByUrl( response.imageUrl, fileName );
    Console.WriteLine( "Task {0} download image from {1} to {2}", num, response.imageUrl, fileName );
}

async Task<Response> GetRandomDogImageUrlAsync( string apiUrl, HttpClient client )
{
    string response = await client.GetStringAsync( ApiUrl );

    JObject json = JObject.Parse( response );

    if ( json == null || json.First == null || json.Last == null || json.First.Last == null || json.Last.Last == null )
    {
        return new Response
        {
            imageUrl = "",
            status = "error"
        };
    }

    return new Response
    {
        imageUrl = json.First.Last.ToString(),
        status = json.Last.Last.ToString()
    };
}

void DownloadImageByUrl( string url, string fileName )
{
    using ( WebClient webClient = new WebClient() )
    {
        webClient.DownloadFile( new Uri( url ), fileName );
    }
}

internal struct Response
{
    public string status;
    public string imageUrl;
}