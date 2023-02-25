using Microsoft.AspNetCore.HttpOverrides;
using Microsoft.AspNetCore.Server.Kestrel.Core;
using System.Net;

var builder = WebApplication.CreateBuilder(args);
// Add services to the container.
builder.Services.AddCors(options =>
{
    options.AddPolicy("CorsPolicy",
        builder => builder.AllowAnyOrigin()
          .AllowAnyMethod()
          .AllowAnyHeader()
          .AllowCredentials()
    .Build());
});
builder.Services.Configure<ForwardedHeadersOptions>(options =>
{
    options.ForwardedHeaders =
        ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto;
});
builder.Services.AddHsts(options =>
{
    options.Preload = true;
    options.IncludeSubDomains = true;
    options.MaxAge = TimeSpan.FromHours(1);

});

builder.Services.Configure<KestrelServerOptions>(options =>
{ 
    options.Listen(IPAddress.Loopback, 5073);
    options.Listen(IPAddress.Loopback, 7073, listenOptions =>
    {

        listenOptions.UseHttps(@"C:\Certs\api.pfx", "$olanaUE5");

    });

});

builder.Services.AddControllers();


var app = builder.Build();


app.UseAuthorization();

app.MapControllers();

app.Run();
