import udav.cli;

auto main(int argc, char** argv) -> int
{
    return udav::UdavCli{}.run(argc, argv);
}
