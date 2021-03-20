# coal-doofensmartz
Group doofensmartz's projects, for Computer Organization and Architecture Lab (CS2610), IITM (JM21).

# First-time Setup
Install the Git client, preferably CLI.

Check if it is installed: `git --version` (on CMD/PowerShell/Bash)

You'll have to configure your username and user email.<br>
Type:<br>
`git config --global user.name "first-name last-name"`<br>
`git config --global user.email "your-email-here@somewhere.com"`<br>

Now navigate to the directory where you want to work with the repository, and clone it there:<br>
`git clone https://github.com/V3D3/coal-doofensmartz.git`

After cloning it, cd into the folder that is just created, and type:<br>
`git status`<br>
to check the status of the repo.

Copy your current Cache-Simulator project files into `cacheman`, then change directory to that folder, and type:<br>
`git add .`

Then:<br>
`git commit -m "Added cache block struct type."`

This will commit the changes to your **local** repository.<br>
Now, push these changes online:<br>
`git push origin master`<br>
Note that you'll have to login to GitHub, use your GitHub credentials.

Or replace the message with something more descriptive, as you see fit.<br>

This guide was a one-time setup.<br>

# Getting changes
To get the latest version of the repository, type:<br>
`git pull origin master`

# Making changes
To upload your code, type:<br>
`git add .`<br>
`git commit -m "<Enter your commit message here>"`<br>
`git push origin master`<br>

The first command adds all files to the staging area.<br>
The second command commits it locally.<br>
The third command syncs these changes to the online repository, so others could get those changes.<br>

# Guidelines
Each folder will contain a separate project, name being the name of the project, and:
- a file `req.txt` that describes the requirements of the project, for convenience, and as per given guidelines.
- a folder `reqsrc` that contains the original assignment PDF, and any related files.
