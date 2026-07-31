/*
This file contains the main() function of the FDF program, along with functions
for checking user input, file validity, and initialization of data structures.
*/

#include "fdf.h"

// IN FILE:

int		main(int argc, char **argv);

//	+++++++++++++++
//	++ FUNCTIONS ++
//	+++++++++++++++

/*
Checks user input and the passed file.
Prints an error message and terminates the program if:
- Not exactly one argument is passed.
- The passed file cannot be opened (e.g., does not exist or no read rights).
- The passed file does not have the '.fdf' extension.
*/
static void	check_file(int argc, char **argv)
{
	int	fd;
	int	len;

	if (argc != 2)
		msg_and_exit(ERR_ARG, NULL);
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		perror_and_exit(argv[1], NULL);
	close (fd);
	len = ft_strlen(argv[1]);
	while (len >= 0 && argv[1][len] != '.')
		len--;
	if (ft_strcmp(argv[1] + len, ".fdf"))
		msg_and_exit(ERR_FILE_TYPE, NULL);
}

/*
Initializes map members in the FDF structure to starting values.
This helps prevent accessing uninitialized variables, particularly
in functions like free_fdf(), which is automatically called in case
of program termination due to an error.
*/
static void	null_fdf(t_fdf *fdf)
{
	fdf->x_max = 0;
	fdf->y_max = 0;
	fdf->z = NULL;
	fdf->color = NULL;
	fdf->col_prov = 0;
	fdf->fd = -1;
	fdf->line = NULL;
	fdf->x_proj_max = 0;
	fdf->y_proj_max = 0;
	fdf->x_proj_min = (float)INT_MAX;
	fdf->y_proj_min = (float)INT_MAX;
	fdf->x_proj = NULL;
	fdf->y_proj = NULL;
	fdf->mlx = NULL;
}

/*
Parses map information into the FDF structure.
- Retrieves map width (fdf->x_max)
- Retrieves map height (fdf->y_max)
- Populates map depth data (fdf->z)
- Populates map color data (fdf->color)
*/
static void	parse_map(t_fdf *fdf, char *file)
{
	get_x_and_y(fdf, file);
	get_z(fdf, file);
	get_color(fdf, file);
}

/*
Initializes the MiniLibX components required for graphic rendering:
- Establishes the connection to the graphic system (fdf->mlx).
- Creates the window (fdf->win).
- Sets up the image buffer (fdf->img).
*/
static void	init_mlx(t_fdf *fdf)
{
	fdf->mlx = mlx_init();
	if (!fdf->mlx)
		msg_and_exit(ERR_MLX, fdf);
	fdf->win = mlx_new_window(fdf->mlx, WINDOW_W, WINDOW_H, WINDOW_TITLE);
	if (!fdf->win)
		msg_and_exit(ERR_MLX, fdf);
	fdf->img.img = mlx_new_image(fdf->mlx, WINDOW_W, WINDOW_H);
	fdf->img.data = mlx_get_data_addr(fdf->img.img, &fdf->img.bpp,
			&fdf->img.size_len, &fdf->img.endian);
}

//	+++++++++++++
//	++ PROGRAM ++
//	+++++++++++++

/*
The FDF program:
- check_file:	Checks the provided file for validity.
- null_fdf:		Initializes the 'fdf' data structure with starting values.
- parse_map:	Parses map information into the data structure.
- init_mlx:		Initializes MiniLibX components required for graphic rendering.
- project_3d_to_2d:			Calculates projected coordinates as seen on image.
- render_map_grid:			Renders map onto the image and displays it in window.
- mlx_key_hook, mlx_hook: 	Establishes hook events for user interaction,
							such as key input and window manipulation.
- mlx_loop:		Starts event handling.
*/
int	main(int argc, char **argv)
{
	t_fdf	fdf;

	check_file(argc, argv);
	null_fdf(&fdf);
	parse_map(&fdf, argv[1]);
	init_mlx(&fdf);
	project_3d_to_2d(&fdf);
	render_map_grid(&fdf);
	mlx_key_hook(fdf.win, (int (*)(int, void *))&handle_keypress, &fdf);
	mlx_hook(fdf.win, DestroyNotify, 0, (int (*)())&handle_x, &fdf);
	mlx_loop(fdf.mlx);
}
